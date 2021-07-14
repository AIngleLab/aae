/*

 */
package org.apache.aingle;

import static org.junit.Assert.*;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema.Type;
import org.apache.aingle.file.DataFileConstants;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.util.Utf8;
import org.junit.Test;

public class TestDataFileCorruption {

  private static final File DIR = new File("/tmp");

  private File makeFile(String name) {
    return new File(DIR, "test-" + name + ".aingle");
  }

  @Test
  public void testCorruptedFile() throws IOException {
    Schema schema = Schema.create(Type.STRING);

    // Write a data file
    DataFileWriter<Utf8> w = new DataFileWriter<>(new GenericDatumWriter<>(schema));
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    w.create(schema, baos);
    w.append(new Utf8("apple"));
    w.append(new Utf8("banana"));
    w.sync();
    w.append(new Utf8("celery"));
    w.append(new Utf8("date"));
    long pos = w.sync();
    w.append(new Utf8("endive"));
    w.append(new Utf8("fig"));
    w.close();

    // Corrupt the input by inserting some zero bytes before the sync marker for the
    // penultimate block
    byte[] original = baos.toByteArray();
    int corruptPosition = (int) pos - DataFileConstants.SYNC_SIZE;
    int corruptedBytes = 3;
    byte[] corrupted = new byte[original.length + corruptedBytes];
    System.arraycopy(original, 0, corrupted, 0, corruptPosition);
    System.arraycopy(original, corruptPosition, corrupted, corruptPosition + corruptedBytes,
        original.length - corruptPosition);

    File file = makeFile("corrupt");
    file.deleteOnExit();
    FileOutputStream out = new FileOutputStream(file);
    out.write(corrupted);
    out.close();

    // Read the data file
    try (DataFileReader r = new DataFileReader<>(file, new GenericDatumReader<>(schema))) {
      assertEquals("apple", r.next().toString());
      assertEquals("banana", r.next().toString());
      long prevSync = r.previousSync();
      r.next();
      fail("Corrupt block should throw exception");
      r.sync(prevSync); // go to sync point after previous successful one
      assertEquals("endive", r.next().toString());
      assertEquals("fig", r.next().toString());
      assertFalse(r.hasNext());
    } catch (AIngleRuntimeException e) {
      assertEquals("Invalid sync!", e.getCause().getMessage());
    }

  }
}
