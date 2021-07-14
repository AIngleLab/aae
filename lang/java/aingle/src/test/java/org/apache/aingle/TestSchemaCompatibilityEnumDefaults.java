/*

 */
package org.apache.aingle;

import static org.apache.aingle.TestSchemas.ENUM1_ABC_SCHEMA;
import static org.apache.aingle.TestSchemas.ENUM1_AB_SCHEMA;
import static org.apache.aingle.TestSchemas.ENUM2_AB_SCHEMA;
import static org.apache.aingle.TestSchemas.ENUM_ABC_ENUM_DEFAULT_A_SCHEMA;
import static org.apache.aingle.TestSchemas.ENUM_AB_ENUM_DEFAULT_A_SCHEMA;
import static org.junit.Assert.assertEquals;

import java.io.ByteArrayOutputStream;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class TestSchemaCompatibilityEnumDefaults {
  @Rule
  public ExpectedException expectedException = ExpectedException.none();

  @Test
  public void testEnumDefaultNotAppliedWhenWriterFieldMissing() throws Exception {
    expectedException.expect(AIngleTypeException.class);
    expectedException.expectMessage("Found Record1, expecting Record1, missing required field field1");

    Schema writerSchema = SchemaBuilder.record("Record1").fields().name("field2").type(ENUM2_AB_SCHEMA).noDefault()
        .endRecord();

    Schema readerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM_AB_ENUM_DEFAULT_A_SCHEMA)
        .noDefault().endRecord();

    GenericRecord datum = new GenericData.Record(writerSchema);
    datum.put("field2", new GenericData.EnumSymbol(writerSchema, "B"));
    serializeWithWriterThenDeserializeWithReader(writerSchema, datum, readerSchema);
  }

  @Test
  public void testEnumDefaultAppliedWhenNoFieldDefaultDefined() throws Exception {
    Schema writerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM_ABC_ENUM_DEFAULT_A_SCHEMA)
        .noDefault().endRecord();

    Schema readerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM_AB_ENUM_DEFAULT_A_SCHEMA)
        .noDefault().endRecord();

    GenericRecord datum = new GenericData.Record(writerSchema);
    datum.put("field1", new GenericData.EnumSymbol(writerSchema, "C"));
    GenericRecord decodedDatum = serializeWithWriterThenDeserializeWithReader(writerSchema, datum, readerSchema);
    // The A is the Enum fallback value.
    assertEquals("A", decodedDatum.get("field1").toString());
  }

  @Test
  public void testEnumDefaultNotAppliedWhenCompatibleSymbolIsFound() throws Exception {
    Schema writerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM_ABC_ENUM_DEFAULT_A_SCHEMA)
        .noDefault().endRecord();

    Schema readerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM_AB_ENUM_DEFAULT_A_SCHEMA)
        .noDefault().endRecord();

    GenericRecord datum = new GenericData.Record(writerSchema);
    datum.put("field1", new GenericData.EnumSymbol(writerSchema, "B"));
    GenericRecord decodedDatum = serializeWithWriterThenDeserializeWithReader(writerSchema, datum, readerSchema);
    assertEquals("B", decodedDatum.get("field1").toString());
  }

  @Test
  public void testEnumDefaultAppliedWhenFieldDefaultDefined() throws Exception {
    Schema writerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM_ABC_ENUM_DEFAULT_A_SCHEMA)
        .noDefault().endRecord();

    Schema readerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM_AB_ENUM_DEFAULT_A_SCHEMA)
        .withDefault("B").endRecord();

    GenericRecord datum = new GenericData.Record(writerSchema);
    datum.put("field1", new GenericData.EnumSymbol(writerSchema, "C"));
    GenericRecord decodedDatum = serializeWithWriterThenDeserializeWithReader(writerSchema, datum, readerSchema);
    // The A is the Enum default, which is assigned since C is not in [A,B].
    assertEquals("A", decodedDatum.get("field1").toString());
  }

  @Test
  public void testFieldDefaultNotAppliedForUnknownSymbol() throws Exception {
    expectedException.expect(AIngleTypeException.class);
    expectedException.expectMessage("No match for C");

    Schema writerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM1_ABC_SCHEMA).noDefault()
        .endRecord();
    Schema readerSchema = SchemaBuilder.record("Record1").fields().name("field1").type(ENUM1_AB_SCHEMA).withDefault("A")
        .endRecord();

    GenericRecord datum = new GenericData.Record(writerSchema);
    datum.put("field1", new GenericData.EnumSymbol(writerSchema, "C"));
    serializeWithWriterThenDeserializeWithReader(writerSchema, datum, readerSchema);
  }

  private GenericRecord serializeWithWriterThenDeserializeWithReader(Schema writerSchema, GenericRecord datum,
      Schema readerSchema) throws Exception {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    Encoder encoder = EncoderFactory.get().binaryEncoder(baos, null);
    DatumWriter<Object> datumWriter = new GenericDatumWriter<>(writerSchema);
    datumWriter.write(datum, encoder);
    encoder.flush();

    byte[] bytes = baos.toByteArray();
    Decoder decoder = DecoderFactory.get().resolvingDecoder(writerSchema, readerSchema,
        DecoderFactory.get().binaryDecoder(bytes, null));
    DatumReader<Object> datumReader = new GenericDatumReader<>(readerSchema);
    return (GenericRecord) datumReader.read(null, decoder);
  }

}
