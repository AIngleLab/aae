/*

 */
package org.apache.trevni.aingle;

import java.io.File;
import java.io.EOFException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.util.List;
import java.util.ArrayList;

import org.apache.trevni.ColumnFileMetaData;

import org.apache.aingle.Schema;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.generic.GenericDatumReader;

import org.junit.Test;
import static org.junit.Assert.*;

public class TestCases {

  private static final File DIR = new File("src/test/cases/");
  private static final File FILE = new File("target", "case.trv");

  @Test
  public void testCases() throws Exception {
    for (File f : DIR.listFiles())
      if (f.isDirectory() && !f.getName().startsWith("."))
        runCase(f);
  }

  private void runCase(File dir) throws Exception {
    Schema schema = new Schema.Parser().parse(new File(dir, "input.ain"));
    List<Object> data = fromJson(schema, new File(dir, "input.json"));

    // write full data
    AIngleColumnWriter<Object> writer = new AIngleColumnWriter<>(schema, new ColumnFileMetaData());
    for (Object datum : data)
      writer.write(datum);
    writer.writeTo(FILE);

    // test that the full schema reads correctly
    checkRead(schema, data);

    // test that sub-schemas read correctly
    for (File f : dir.listFiles())
      if (f.isDirectory() && !f.getName().startsWith(".")) {
        Schema s = new Schema.Parser().parse(new File(f, "sub.ain"));
        checkRead(s, fromJson(s, new File(f, "sub.json")));
      }
  }

  private void checkRead(Schema s, List<Object> data) throws Exception {
    try (AIngleColumnReader<Object> reader = new AIngleColumnReader<>(new AIngleColumnReader.Params(FILE).setSchema(s))) {
      for (Object datum : data)
        assertEquals(datum, reader.next());
    }
  }

  private List<Object> fromJson(Schema schema, File file) throws Exception {
    List<Object> data = new ArrayList<>();
    try (InputStream in = new FileInputStream(file)) {
      DatumReader reader = new GenericDatumReader(schema);
      Decoder decoder = DecoderFactory.get().jsonDecoder(schema, in);
      while (true)
        data.add(reader.read(null, decoder));
    } catch (EOFException e) {
    }
    return data;
  }

}
