/*

 */
package org.apache.aingle.generic;

import org.apache.aingle.FooBarSpecificRecord;
import org.apache.aingle.TypeEnum;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.specific.SpecificDatumReader;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Collections;

import static org.junit.Assert.assertEquals;

/**
 * See AINGLE-1810: GenericDatumWriter broken with Enum
 */
public class TestGenericConcreteEnum {

  private static byte[] serializeRecord(FooBarSpecificRecord fooBarSpecificRecord) throws IOException {
    GenericDatumWriter<FooBarSpecificRecord> datumWriter = new GenericDatumWriter<>(FooBarSpecificRecord.SCHEMA$);
    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
    Encoder encoder = EncoderFactory.get().binaryEncoder(byteArrayOutputStream, null);
    datumWriter.write(fooBarSpecificRecord, encoder);
    encoder.flush();
    return byteArrayOutputStream.toByteArray();
  }

  @Test
  public void testGenericWriteAndRead() throws IOException {
    FooBarSpecificRecord specificRecord = getRecord();

    byte[] bytes = serializeRecord(specificRecord);

    Decoder decoder = DecoderFactory.get().binaryDecoder(bytes, null);

    GenericDatumReader<IndexedRecord> genericDatumReader = new GenericDatumReader<>(FooBarSpecificRecord.SCHEMA$);
    IndexedRecord deserialized = new GenericData.Record(FooBarSpecificRecord.SCHEMA$);
    genericDatumReader.read(deserialized, decoder);

    assertEquals(0, GenericData.get().compare(specificRecord, deserialized, FooBarSpecificRecord.SCHEMA$));
  }

  @Test
  public void testGenericWriteSpecificRead() throws IOException {
    FooBarSpecificRecord specificRecord = getRecord();

    byte[] bytes = serializeRecord(specificRecord);

    Decoder decoder = DecoderFactory.get().binaryDecoder(bytes, null);

    SpecificDatumReader<FooBarSpecificRecord> specificDatumReader = new SpecificDatumReader<>(
        FooBarSpecificRecord.SCHEMA$);
    FooBarSpecificRecord deserialized = new FooBarSpecificRecord();
    specificDatumReader.read(deserialized, decoder);

    assertEquals(specificRecord, deserialized);
  }

  private FooBarSpecificRecord getRecord() {
    return FooBarSpecificRecord.newBuilder().setId(42).setName("foo").setNicknames(Collections.singletonList("bar"))
        .setRelatedids(Collections.singletonList(3)).setTypeEnum(TypeEnum.a).build();
  }
}
