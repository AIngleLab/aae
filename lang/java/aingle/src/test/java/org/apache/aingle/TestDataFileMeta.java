/*

 */
package org.apache.aingle;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import org.apache.aingle.Schema.Type;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestDataFileMeta {

  @Rule
  public TemporaryFolder DIR = new TemporaryFolder();

  @Test(expected = AIngleRuntimeException.class)
  public void testUseReservedMeta() throws IOException {
    try (DataFileWriter<?> w = new DataFileWriter<>(new GenericDatumWriter<>())) {
      w.setMeta("aingle.foo", "bar");
    }
  }

  @Test()
  public void testUseMeta() throws IOException {
    File f = new File(DIR.getRoot().getPath(), "testDataFileMeta.aingle");
    try (DataFileWriter<?> w = new DataFileWriter<>(new GenericDatumWriter<>())) {
      w.setMeta("hello", "bar");
      w.create(Schema.create(Type.NULL), f);
    }

    try (DataFileStream<Void> r = new DataFileStream<>(new FileInputStream(f), new GenericDatumReader<>())) {
      assertTrue(r.getMetaKeys().contains("hello"));

      assertEquals("bar", r.getMetaString("hello"));
    }

  }

  @Test(expected = AIngleRuntimeException.class)
  public void testUseMetaAfterCreate() throws IOException {
    try (DataFileWriter<?> w = new DataFileWriter<>(new GenericDatumWriter<>())) {
      w.create(Schema.create(Type.NULL), new ByteArrayOutputStream());
      w.setMeta("foo", "bar");
    }

  }

  @Test
  public void testBlockSizeSetInvalid() {
    int exceptions = 0;
    for (int i = -1; i < 33; i++) {
      // 33 invalid, one valid
      try {
        new DataFileWriter<>(new GenericDatumWriter<>()).setSyncInterval(i);
      } catch (IllegalArgumentException iae) {
        exceptions++;
      }
    }
    assertEquals(33, exceptions);
  }
}
