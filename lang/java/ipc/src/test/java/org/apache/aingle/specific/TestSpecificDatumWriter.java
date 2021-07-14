/*

 */
package org.apache.aingle.specific;

import static org.junit.Assert.assertEquals;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.io.JsonEncoder;
import org.apache.aingle.test.Kind;
import org.apache.aingle.test.TestRecordWithUnion;
import org.junit.Test;

public class TestSpecificDatumWriter {
  @Test
  public void testResolveUnion() throws IOException {
    final SpecificDatumWriter<TestRecordWithUnion> writer = new SpecificDatumWriter<>();
    Schema schema = TestRecordWithUnion.SCHEMA$;
    ByteArrayOutputStream out = new ByteArrayOutputStream();
    JsonEncoder encoder = EncoderFactory.get().jsonEncoder(schema, out);

    writer.setSchema(schema);

    TestRecordWithUnion c = TestRecordWithUnion.newBuilder().setKind(Kind.BAR).setValue("rab").build();
    writer.write(c, encoder);
    encoder.flush();
    out.close();

    String expectedJson = String.format("{'kind':{'org.apache.aingle.test.Kind':'%s'},'value':{'string':'%s'}}",
        c.getKind().toString(), c.getValue()).replace('\'', '"');

    assertEquals(expectedJson, out.toString("UTF-8"));
  }

}
