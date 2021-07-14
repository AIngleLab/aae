/*

 */
package org.apache.aingle.io;

import com.fasterxml.jackson.core.JsonEncoding;
import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.apache.aingle.AIngleTypeException;
import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

import static java.util.Arrays.asList;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.Assert.assertThat;

public class TestEncoders {
  private static final int ENCODER_BUFFER_SIZE = 32;
  private static final int EXAMPLE_DATA_SIZE = 17;

  private static EncoderFactory factory = EncoderFactory.get();

  @Rule
  public TemporaryFolder DIR = new TemporaryFolder();

  @Test
  public void testBinaryEncoderInit() throws IOException {
    OutputStream out = new ByteArrayOutputStream();
    BinaryEncoder enc = factory.binaryEncoder(out, null);
    Assert.assertSame(enc, factory.binaryEncoder(out, enc));
  }

  @Test(expected = NullPointerException.class)
  public void testBadBinaryEncoderInit() {
    factory.binaryEncoder(null, null);
  }

  @Test
  public void testBlockingBinaryEncoderInit() throws IOException {
    OutputStream out = new ByteArrayOutputStream();
    BinaryEncoder reuse = null;
    reuse = factory.blockingBinaryEncoder(out, reuse);
    Assert.assertSame(reuse, factory.blockingBinaryEncoder(out, reuse));
    // comparison
  }

  @Test(expected = NullPointerException.class)
  public void testBadBlockintBinaryEncoderInit() {
    factory.binaryEncoder(null, null);
  }

  @Test
  public void testDirectBinaryEncoderInit() throws IOException {
    OutputStream out = new ByteArrayOutputStream();
    BinaryEncoder enc = factory.directBinaryEncoder(out, null);
    Assert.assertSame(enc, factory.directBinaryEncoder(out, enc));
  }

  @Test(expected = NullPointerException.class)
  public void testBadDirectBinaryEncoderInit() {
    factory.directBinaryEncoder(null, null);
  }

  @Test
  public void testJsonEncoderInit() throws IOException {
    Schema s = new Schema.Parser().parse("\"int\"");
    OutputStream out = new ByteArrayOutputStream();
    factory.jsonEncoder(s, out);
    JsonEncoder enc = factory.jsonEncoder(s, new JsonFactory().createGenerator(out, JsonEncoding.UTF8));
    enc.configure(out);
  }

  @Test(expected = NullPointerException.class)
  public void testBadJsonEncoderInitOS() throws IOException {
    factory.jsonEncoder(Schema.create(Type.INT), (OutputStream) null);
  }

  @Test(expected = NullPointerException.class)
  public void testBadJsonEncoderInit() throws IOException {
    factory.jsonEncoder(Schema.create(Type.INT), (JsonGenerator) null);
  }

  @Test
  public void testJsonEncoderNewlineDelimited() throws IOException {
    OutputStream out = new ByteArrayOutputStream();
    Schema ints = Schema.create(Type.INT);
    Encoder e = factory.jsonEncoder(ints, out);
    String separator = System.getProperty("line.separator");
    GenericDatumWriter<Integer> writer = new GenericDatumWriter<>(ints);
    writer.write(1, e);
    writer.write(2, e);
    e.flush();
    Assert.assertEquals("1" + separator + "2", out.toString());
  }

  @Test
  public void testJsonEncoderWhenIncludeNamespaceOptionIsFalse() throws IOException {
    String value = "{\"b\": {\"string\":\"myVal\"}, \"a\": 1}";
    String schemaStr = "{\"type\": \"record\", \"name\": \"ab\", \"fields\": ["
        + "{\"name\": \"a\", \"type\": \"int\"}, {\"name\": \"b\", \"type\": [\"null\", \"string\"]}" + "]}";
    Schema schema = new Schema.Parser().parse(schemaStr);
    byte[] aingleBytes = fromJsonToAIngle(value, schema);
    ObjectMapper mapper = new ObjectMapper();

    Assert.assertEquals(mapper.readTree("{\"b\":\"myVal\",\"a\":1}"),
        mapper.readTree(fromAIngleToJson(aingleBytes, schema, false)));
  }

  @Test
  public void testJsonEncoderWhenIncludeNamespaceOptionIsTrue() throws IOException {
    String value = "{\"b\": {\"string\":\"myVal\"}, \"a\": 1}";
    String schemaStr = "{\"type\": \"record\", \"name\": \"ab\", \"fields\": ["
        + "{\"name\": \"a\", \"type\": \"int\"}, {\"name\": \"b\", \"type\": [\"null\", \"string\"]}" + "]}";
    Schema schema = new Schema.Parser().parse(schemaStr);
    byte[] aingleBytes = fromJsonToAIngle(value, schema);
    ObjectMapper mapper = new ObjectMapper();

    Assert.assertEquals(mapper.readTree("{\"b\":{\"string\":\"myVal\"},\"a\":1}"),
        mapper.readTree(fromAIngleToJson(aingleBytes, schema, true)));
  }

  @Test
  public void testValidatingEncoderInit() throws IOException {
    Schema s = new Schema.Parser().parse("\"int\"");
    OutputStream out = new ByteArrayOutputStream();
    Encoder e = factory.directBinaryEncoder(out, null);
    factory.validatingEncoder(s, e).configure(e);
  }

  @Test
  public void testJsonRecordOrdering() throws IOException {
    String value = "{\"b\": 2, \"a\": 1}";
    Schema schema = new Schema.Parser().parse("{\"type\": \"record\", \"name\": \"ab\", \"fields\": ["
        + "{\"name\": \"a\", \"type\": \"int\"}, {\"name\": \"b\", \"type\": \"int\"}" + "]}");
    GenericDatumReader<Object> reader = new GenericDatumReader<>(schema);
    Decoder decoder = DecoderFactory.get().jsonDecoder(schema, value);
    Object o = reader.read(null, decoder);
    Assert.assertEquals("{\"a\": 1, \"b\": 2}", o.toString());
  }

  @Test(expected = AIngleTypeException.class)
  public void testJsonExcessFields() throws IOException {
    String value = "{\"b\": { \"b3\": 1.4, \"b2\": 3.14, \"b1\": \"h\"}, \"a\": {\"a0\": 45, \"a2\":true, \"a1\": null}}";
    Schema schema = new Schema.Parser().parse("{\"type\": \"record\", \"name\": \"ab\", \"fields\": [\n"
        + "{\"name\": \"a\", \"type\": {\"type\":\"record\",\"name\":\"A\",\"fields\":\n"
        + "[{\"name\":\"a1\", \"type\":\"null\"}, {\"name\":\"a2\", \"type\":\"boolean\"}]}},\n"
        + "{\"name\": \"b\", \"type\": {\"type\":\"record\",\"name\":\"B\",\"fields\":\n"
        + "[{\"name\":\"b1\", \"type\":\"string\"}, {\"name\":\"b2\", \"type\":\"float\"}, {\"name\":\"b3\", \"type\":\"double\"}]}}\n"
        + "]}");
    GenericDatumReader<Object> reader = new GenericDatumReader<>(schema);
    Decoder decoder = DecoderFactory.get().jsonDecoder(schema, value);
    reader.read(null, decoder);
  }

  @Test
  public void testJsonRecordOrdering2() throws IOException {
    String value = "{\"b\": { \"b3\": 1.4, \"b2\": 3.14, \"b1\": \"h\"}, \"a\": {\"a2\":true, \"a1\": null}}";
    Schema schema = new Schema.Parser().parse("{\"type\": \"record\", \"name\": \"ab\", \"fields\": [\n"
        + "{\"name\": \"a\", \"type\": {\"type\":\"record\",\"name\":\"A\",\"fields\":\n"
        + "[{\"name\":\"a1\", \"type\":\"null\"}, {\"name\":\"a2\", \"type\":\"boolean\"}]}},\n"
        + "{\"name\": \"b\", \"type\": {\"type\":\"record\",\"name\":\"B\",\"fields\":\n"
        + "[{\"name\":\"b1\", \"type\":\"string\"}, {\"name\":\"b2\", \"type\":\"float\"}, {\"name\":\"b3\", \"type\":\"double\"}]}}\n"
        + "]}");
    GenericDatumReader<Object> reader = new GenericDatumReader<>(schema);
    Decoder decoder = DecoderFactory.get().jsonDecoder(schema, value);
    Object o = reader.read(null, decoder);
    Assert.assertEquals("{\"a\": {\"a1\": null, \"a2\": true}, \"b\": {\"b1\": \"h\", \"b2\": 3.14, \"b3\": 1.4}}",
        o.toString());
  }

  @Test
  public void testJsonRecordOrderingWithProjection() throws IOException {
    String value = "{\"b\": { \"b3\": 1.4, \"b2\": 3.14, \"b1\": \"h\"}, \"a\": {\"a2\":true, \"a1\": null}}";
    Schema writerSchema = new Schema.Parser().parse("{\"type\": \"record\", \"name\": \"ab\", \"fields\": [\n"
        + "{\"name\": \"a\", \"type\": {\"type\":\"record\",\"name\":\"A\",\"fields\":\n"
        + "[{\"name\":\"a1\", \"type\":\"null\"}, {\"name\":\"a2\", \"type\":\"boolean\"}]}},\n"
        + "{\"name\": \"b\", \"type\": {\"type\":\"record\",\"name\":\"B\",\"fields\":\n"
        + "[{\"name\":\"b1\", \"type\":\"string\"}, {\"name\":\"b2\", \"type\":\"float\"}, {\"name\":\"b3\", \"type\":\"double\"}]}}\n"
        + "]}");
    Schema readerSchema = new Schema.Parser().parse("{\"type\": \"record\", \"name\": \"ab\", \"fields\": [\n"
        + "{\"name\": \"a\", \"type\": {\"type\":\"record\",\"name\":\"A\",\"fields\":\n"
        + "[{\"name\":\"a1\", \"type\":\"null\"}, {\"name\":\"a2\", \"type\":\"boolean\"}]}}\n" + "]}");
    GenericDatumReader<Object> reader = new GenericDatumReader<>(writerSchema, readerSchema);
    Decoder decoder = DecoderFactory.get().jsonDecoder(writerSchema, value);
    Object o = reader.read(null, decoder);
    Assert.assertEquals("{\"a\": {\"a1\": null, \"a2\": true}}", o.toString());
  }

  @Test
  public void testJsonRecordOrderingWithProjection2() throws IOException {
    String value = "{\"b\": { \"b1\": \"h\", \"b2\": [3.14, 3.56], \"b3\": 1.4}, \"a\": {\"a2\":true, \"a1\": null}}";
    Schema writerSchema = new Schema.Parser().parse("{\"type\": \"record\", \"name\": \"ab\", \"fields\": [\n"
        + "{\"name\": \"a\", \"type\": {\"type\":\"record\",\"name\":\"A\",\"fields\":\n"
        + "[{\"name\":\"a1\", \"type\":\"null\"}, {\"name\":\"a2\", \"type\":\"boolean\"}]}},\n"
        + "{\"name\": \"b\", \"type\": {\"type\":\"record\",\"name\":\"B\",\"fields\":\n"
        + "[{\"name\":\"b1\", \"type\":\"string\"}, {\"name\":\"b2\", \"type\":{\"type\":\"array\", \"items\":\"float\"}}, {\"name\":\"b3\", \"type\":\"double\"}]}}\n"
        + "]}");
    Schema readerSchema = new Schema.Parser().parse("{\"type\": \"record\", \"name\": \"ab\", \"fields\": [\n"
        + "{\"name\": \"a\", \"type\": {\"type\":\"record\",\"name\":\"A\",\"fields\":\n"
        + "[{\"name\":\"a1\", \"type\":\"null\"}, {\"name\":\"a2\", \"type\":\"boolean\"}]}}\n" + "]}");
    GenericDatumReader<Object> reader = new GenericDatumReader<>(writerSchema, readerSchema);
    Decoder decoder = DecoderFactory.get().jsonDecoder(writerSchema, value);
    Object o = reader.read(null, decoder);
    Assert.assertEquals("{\"a\": {\"a1\": null, \"a2\": true}}", o.toString());
  }

  @Test
  public void testArrayBackedByteBuffer() throws IOException {
    ByteBuffer buffer = ByteBuffer.wrap(someBytes(EXAMPLE_DATA_SIZE));

    testWithBuffer(buffer);
  }

  @Test
  public void testMappedByteBuffer() throws IOException {
    Path file = Paths.get(DIR.getRoot().getPath() + "testMappedByteBuffer.aingle");
    Files.write(file, someBytes(EXAMPLE_DATA_SIZE));
    MappedByteBuffer buffer = FileChannel.open(file, StandardOpenOption.READ).map(FileChannel.MapMode.READ_ONLY, 0,
        EXAMPLE_DATA_SIZE);

    testWithBuffer(buffer);
  }

  private void testWithBuffer(ByteBuffer buffer) throws IOException {
    assertThat(asList(buffer.position(), buffer.remaining()), is(asList(0, EXAMPLE_DATA_SIZE)));

    ByteArrayOutputStream output = new ByteArrayOutputStream(EXAMPLE_DATA_SIZE * 2);
    EncoderFactory encoderFactory = new EncoderFactory();
    encoderFactory.configureBufferSize(ENCODER_BUFFER_SIZE);

    Encoder encoder = encoderFactory.binaryEncoder(output, null);
    new GenericDatumWriter<ByteBuffer>(Schema.create(Schema.Type.BYTES)).write(buffer, encoder);
    encoder.flush();

    assertThat(output.toByteArray(), equalTo(aingleEncoded(someBytes(EXAMPLE_DATA_SIZE))));
    assertThat(asList(buffer.position(), buffer.remaining()), is(asList(0, EXAMPLE_DATA_SIZE))); // fails if buffer is
    // not array-backed and
    // buffer overflow
    // occurs
  }

  private byte[] someBytes(int size) {
    byte[] result = new byte[size];
    for (int i = 0; i < size; i++) {
      result[i] = (byte) i;
    }
    return result;
  }

  private byte[] aingleEncoded(byte[] bytes) {
    assert bytes.length < 64;
    byte[] result = new byte[1 + bytes.length];
    result[0] = (byte) (bytes.length * 2); // zig-zag encoding
    System.arraycopy(bytes, 0, result, 1, bytes.length);
    return result;
  }

  private byte[] fromJsonToAIngle(String json, Schema schema) throws IOException {
    DatumReader<Object> reader = new GenericDatumReader<>(schema);
    GenericDatumWriter<Object> writer = new GenericDatumWriter<>(schema);
    ByteArrayOutputStream output = new ByteArrayOutputStream();

    Decoder decoder = DecoderFactory.get().jsonDecoder(schema, json);
    Encoder encoder = EncoderFactory.get().binaryEncoder(output, null);

    Object datum = reader.read(null, decoder);

    writer.write(datum, encoder);
    encoder.flush();

    return output.toByteArray();
  }

  private String fromAIngleToJson(byte[] aingleBytes, Schema schema, boolean includeNamespace) throws IOException {
    GenericDatumReader<Object> reader = new GenericDatumReader<>(schema);
    DatumWriter<Object> writer = new GenericDatumWriter<>(schema);
    ByteArrayOutputStream output = new ByteArrayOutputStream();

    JsonEncoder encoder = factory.jsonEncoder(schema, output);
    encoder.setIncludeNamespace(includeNamespace);
    Decoder decoder = DecoderFactory.get().binaryDecoder(aingleBytes, null);
    Object datum = reader.read(null, decoder);
    writer.write(datum, encoder);
    encoder.flush();
    output.flush();

    return new String(output.toByteArray(), StandardCharsets.UTF_8.name());
  }
}
