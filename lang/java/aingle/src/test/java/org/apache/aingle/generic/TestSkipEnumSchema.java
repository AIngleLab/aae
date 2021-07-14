/*

 */
package org.apache.aingle.generic;

import org.apache.aingle.Schema;
import org.apache.aingle.SchemaBuilder;
import org.apache.aingle.generic.GenericData.EnumSymbol;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * See AINGLE-2908
 */
public class TestSkipEnumSchema {
  @Test
  public void testSkipEnum() throws IOException {
    Schema enumSchema = SchemaBuilder.builder().enumeration("enum").symbols("en1", "en2");
    EnumSymbol enumSymbol = new EnumSymbol(enumSchema, "en1");

    GenericDatumWriter<EnumSymbol> datumWriter = new GenericDatumWriter<>(enumSchema);
    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
    Encoder encoder = EncoderFactory.get().validatingEncoder(enumSchema,
        EncoderFactory.get().binaryEncoder(byteArrayOutputStream, null));
    datumWriter.write(enumSymbol, encoder);
    encoder.flush();

    Decoder decoder = DecoderFactory.get().validatingDecoder(enumSchema,
        DecoderFactory.get().binaryDecoder(byteArrayOutputStream.toByteArray(), null));

    GenericDatumReader.skip(enumSchema, decoder);
  }
}
