/*

 */
package org.apache.aingle;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema.Type;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.util.Utf8;
import org.junit.Test;

/** Simple test of DataFileWriter and DataFileStream with deflate codec. */
public class TestDataFileDeflate {
  @Test
  public void testWriteAndRead() throws IOException {
    Schema schema = Schema.create(Type.STRING);

    // Write it
    DataFileWriter<Utf8> w = new DataFileWriter<>(new GenericDatumWriter<>(schema));
    w.setCodec(CodecFactory.deflateCodec(6));
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    w.create(schema, baos);
    w.append(new Utf8("hello world"));
    w.append(new Utf8("hello moon"));
    w.sync();
    w.append(new Utf8("bye bye world"));
    w.append(new Utf8("bye bye moon"));
    w.close();

    // Read it
    try (DataFileStream<Utf8> r = new DataFileStream<>(new ByteArrayInputStream(baos.toByteArray()),
        new GenericDatumReader<>(schema))) {
      assertEquals("hello world", r.next().toString());
      assertEquals("hello moon", r.next().toString());
      assertEquals("bye bye world", r.next().toString());
      assertEquals("bye bye moon", r.next().toString());
      assertFalse(r.hasNext());
    }
  }
}
