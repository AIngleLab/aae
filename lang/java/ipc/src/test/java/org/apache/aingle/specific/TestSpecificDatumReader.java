/*

 */

package org.apache.aingle.specific;

import static org.junit.Assert.*;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;

import org.apache.aingle.FooBarSpecificRecord;
import org.apache.aingle.FooBarSpecificRecord.Builder;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.junit.Test;

import test.StringablesRecord;

public class TestSpecificDatumReader {

  public static byte[] serializeRecord(FooBarSpecificRecord fooBarSpecificRecord) throws IOException {
    SpecificDatumWriter<FooBarSpecificRecord> datumWriter = new SpecificDatumWriter<>(FooBarSpecificRecord.SCHEMA$);
    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
    Encoder encoder = EncoderFactory.get().binaryEncoder(byteArrayOutputStream, null);
    datumWriter.write(fooBarSpecificRecord, encoder);
    encoder.flush();
    return byteArrayOutputStream.toByteArray();
  }

  public static byte[] serializeRecord(StringablesRecord stringablesRecord) throws IOException {
    SpecificDatumWriter<StringablesRecord> datumWriter = new SpecificDatumWriter<>(StringablesRecord.SCHEMA$);
    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
    Encoder encoder = EncoderFactory.get().binaryEncoder(byteArrayOutputStream, null);
    datumWriter.write(stringablesRecord, encoder);
    encoder.flush();
    return byteArrayOutputStream.toByteArray();
  }

  @Test
  public void testRead() throws IOException {
    Builder newBuilder = FooBarSpecificRecord.newBuilder();
    newBuilder.setId(42);
    newBuilder.setName("foo");
    newBuilder.setNicknames(Collections.singletonList("bar"));
    newBuilder.setRelatedids(Arrays.asList(1, 2, 3));
    FooBarSpecificRecord specificRecord = newBuilder.build();

    byte[] recordBytes = serializeRecord(specificRecord);

    Decoder decoder = DecoderFactory.get().binaryDecoder(recordBytes, null);
    SpecificDatumReader<FooBarSpecificRecord> specificDatumReader = new SpecificDatumReader<>(
        FooBarSpecificRecord.SCHEMA$);
    FooBarSpecificRecord deserialized = new FooBarSpecificRecord();
    specificDatumReader.read(deserialized, decoder);

    assertEquals(specificRecord, deserialized);
  }

  @Test
  public void testStringables() throws IOException {
    StringablesRecord.Builder newBuilder = StringablesRecord.newBuilder();
    newBuilder.setValue(new BigDecimal("42.11"));
    HashMap<String, BigDecimal> mapWithBigDecimalElements = new HashMap<>();
    mapWithBigDecimalElements.put("test", new BigDecimal("11.11"));
    newBuilder.setMapWithBigDecimalElements(mapWithBigDecimalElements);
    HashMap<BigInteger, String> mapWithBigIntKeys = new HashMap<>();
    mapWithBigIntKeys.put(BigInteger.ONE, "test");
    newBuilder.setMapWithBigIntKeys(mapWithBigIntKeys);
    StringablesRecord stringablesRecord = newBuilder.build();

    byte[] recordBytes = serializeRecord(stringablesRecord);

    Decoder decoder = DecoderFactory.get().binaryDecoder(recordBytes, null);
    SpecificDatumReader<StringablesRecord> specificDatumReader = new SpecificDatumReader<>(StringablesRecord.SCHEMA$);
    StringablesRecord deserialized = new StringablesRecord();
    specificDatumReader.read(deserialized, decoder);

    assertEquals(stringablesRecord, deserialized);

  }

}
