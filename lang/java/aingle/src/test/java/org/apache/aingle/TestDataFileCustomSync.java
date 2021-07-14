/*

 */
package org.apache.aingle;

import static java.nio.charset.StandardCharsets.UTF_8;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.UUID;

import org.apache.aingle.Schema.Type;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.util.Utf8;
import org.junit.Test;

public class TestDataFileCustomSync {
  private byte[] createDataFile(byte[] sync) throws IOException {
    Schema schema = Schema.create(Type.STRING);
    DataFileWriter<Utf8> w = new DataFileWriter<>(new GenericDatumWriter<>(schema));
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    w.create(schema, baos, sync);
    w.append(new Utf8("apple"));
    w.append(new Utf8("banana"));
    w.sync();
    w.append(new Utf8("celery"));
    w.append(new Utf8("date"));
    w.sync();
    w.append(new Utf8("endive"));
    w.append(new Utf8("fig"));
    w.close();
    return baos.toByteArray();
  }

  private static byte[] generateSync() {
    try {
      MessageDigest digester = MessageDigest.getInstance("MD5");
      long time = System.currentTimeMillis();
      digester.update((UUID.randomUUID() + "@" + time).getBytes(UTF_8));
      return digester.digest();
    } catch (NoSuchAlgorithmException e) {
      throw new RuntimeException(e);
    }
  }

  @Test(expected = IOException.class)
  public void testInvalidSync() throws IOException {
    // Invalid size (must be 16):
    byte[] sync = new byte[8];
    createDataFile(sync);
  }

  @Test
  public void testRandomSync() throws IOException {
    byte[] sync = generateSync();
    byte[] randSyncFile = createDataFile(null);
    byte[] customSyncFile = createDataFile(sync);
    assertFalse(Arrays.equals(randSyncFile, customSyncFile));
  }

  @Test
  public void testCustomSync() throws IOException {
    byte[] sync = generateSync();
    byte[] customSyncFile = createDataFile(sync);
    byte[] sameCustomSyncFile = createDataFile(sync);
    assertTrue(Arrays.equals(customSyncFile, sameCustomSyncFile));
  }
}
