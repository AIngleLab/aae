/*

 */
package org.apache.aingle.io.parsing;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.StringReader;
import java.util.Arrays;
import java.util.Collection;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.aingle.AIngleTypeException;
import org.apache.aingle.Schema;
import org.apache.aingle.SchemaBuilder;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.generic.GenericRecordBuilder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class TestResolvingGrammarGenerator {
  private final Schema schema;
  private final JsonNode data;

  public TestResolvingGrammarGenerator(String jsonSchema, String jsonData) throws IOException {
    this.schema = new Schema.Parser().parse(jsonSchema);
    JsonFactory factory = new JsonFactory();
    ObjectMapper mapper = new ObjectMapper(factory);

    this.data = mapper.readTree(new StringReader(jsonData));
  }

  @Test
  public void test() throws IOException {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    EncoderFactory factory = EncoderFactory.get();
    Encoder e = factory.validatingEncoder(schema, factory.binaryEncoder(baos, null));

    ResolvingGrammarGenerator.encode(e, schema, data);
    e.flush();
  }

  @Test
  public void testRecordMissingRequiredFieldError() throws Exception {
    Schema schemaWithoutField = SchemaBuilder.record("MyRecord").namespace("ns").fields().name("field1").type()
        .stringType().noDefault().endRecord();
    Schema schemaWithField = SchemaBuilder.record("MyRecord").namespace("ns").fields().name("field1").type()
        .stringType().noDefault().name("field2").type().stringType().noDefault().endRecord();
    GenericData.Record record = new GenericRecordBuilder(schemaWithoutField).set("field1", "someValue").build();
    byte[] data = writeRecord(schemaWithoutField, record);
    try {
      readRecord(schemaWithField, data);
      Assert.fail("Expected exception not thrown");
    } catch (AIngleTypeException typeException) {
      Assert.assertEquals("Incorrect exception message",
          "Found ns.MyRecord, expecting ns.MyRecord, missing required field field2", typeException.getMessage());
    }
  }

  @Parameterized.Parameters
  public static Collection<Object[]> data() {
    Collection<Object[]> ret = Arrays.asList(new Object[][] {
        { "{ \"type\": \"record\", \"name\": \"r\", \"fields\": [ " + " { \"name\" : \"f1\", \"type\": \"int\" }, "
            + " { \"name\" : \"f2\", \"type\": \"float\" } " + "] } }", "{ \"f2\": 10.4, \"f1\": 10 } " },
        { "{ \"type\": \"enum\", \"name\": \"e\", \"symbols\": " + "[ \"s1\", \"s2\"] } }", " \"s1\" " },
        { "{ \"type\": \"enum\", \"name\": \"e\", \"symbols\": " + "[ \"s1\", \"s2\"] } }", " \"s2\" " },
        { "{ \"type\": \"fixed\", \"name\": \"f\", \"size\": 10 }", "\"hello\"" },
        { "{ \"type\": \"array\", \"items\": \"int\" }", "[ 10, 20, 30 ]" },
        { "{ \"type\": \"map\", \"values\": \"int\" }", "{ \"k1\": 10, \"k3\": 20, \"k3\": 30 }" },
        { "[ \"int\", \"long\" ]", "10" }, { "\"string\"", "\"hello\"" }, { "\"bytes\"", "\"hello\"" },
        { "\"int\"", "10" }, { "\"long\"", "10" }, { "\"float\"", "10.0" }, { "\"double\"", "10.0" },
        { "\"boolean\"", "true" }, { "\"boolean\"", "false" }, { "\"null\"", "null" }, });
    return ret;
  }

  private byte[] writeRecord(Schema schema, GenericData.Record record) throws Exception {
    ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
    GenericDatumWriter<GenericData.Record> datumWriter = new GenericDatumWriter<>(schema);
    try (DataFileWriter<GenericData.Record> writer = new DataFileWriter<>(datumWriter)) {
      writer.create(schema, byteStream);
      writer.append(record);
    }
    return byteStream.toByteArray();
  }

  private GenericData.Record readRecord(Schema schema, byte[] data) throws Exception {
    ByteArrayInputStream byteStream = new ByteArrayInputStream(data);
    GenericDatumReader<GenericData.Record> datumReader = new GenericDatumReader<>(schema);
    try (DataFileStream<GenericData.Record> reader = new DataFileStream<>(byteStream, datumReader)) {
      return reader.next();
    }
  }
}
