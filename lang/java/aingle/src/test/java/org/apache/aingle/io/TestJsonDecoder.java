/*

 */
package org.apache.aingle.io;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericRecord;
import org.junit.Assert;
import org.junit.Test;

public class TestJsonDecoder {

  @Test
  public void testInt() throws Exception {
    checkNumeric("int", 1);
  }

  @Test
  public void testLong() throws Exception {
    checkNumeric("long", 1L);
  }

  @Test
  public void testFloat() throws Exception {
    checkNumeric("float", 1.0F);
  }

  @Test
  public void testDouble() throws Exception {
    checkNumeric("double", 1.0);
  }

  private void checkNumeric(String type, Object value) throws Exception {
    String def = "{\"type\":\"record\",\"name\":\"X\",\"fields\":" + "[{\"type\":\"" + type + "\",\"name\":\"n\"}]}";
    Schema schema = new Schema.Parser().parse(def);
    DatumReader<GenericRecord> reader = new GenericDatumReader<>(schema);

    String[] records = { "{\"n\":1}", "{\"n\":1.0}" };

    for (String record : records) {
      Decoder decoder = DecoderFactory.get().jsonDecoder(schema, record);
      GenericRecord r = reader.read(null, decoder);
      Assert.assertEquals(value, r.get("n"));
    }
  }

  // Ensure that even if the order of fields in JSON is different from the order
  // in schema,
  // it works.
  @Test
  public void testReorderFields() throws Exception {
    String w = "{\"type\":\"record\",\"name\":\"R\",\"fields\":" + "[{\"type\":\"long\",\"name\":\"l\"},"
        + "{\"type\":{\"type\":\"array\",\"items\":\"int\"},\"name\":\"a\"}" + "]}";
    Schema ws = new Schema.Parser().parse(w);
    DecoderFactory df = DecoderFactory.get();
    String data = "{\"a\":[1,2],\"l\":100}{\"l\": 200, \"a\":[1,2]}";
    JsonDecoder in = df.jsonDecoder(ws, data);
    Assert.assertEquals(100, in.readLong());
    in.skipArray();
    Assert.assertEquals(200, in.readLong());
    in.skipArray();
  }
}
