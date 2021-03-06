/*

 */
package org.apache.aingle.specific;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonParser;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import org.apache.aingle.FooBarSpecificRecord;
import org.apache.aingle.Schema;
import org.apache.aingle.TestSchema;
import org.apache.aingle.TypeEnum;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.test.Kind;
import org.apache.aingle.test.MD5;
import org.apache.aingle.test.Reserved;
import org.apache.aingle.test.TestRecord;
import org.junit.Assert;
import org.junit.Test;

public class TestSpecificData {

  @Test
  /** Make sure that even with nulls, hashCode() doesn't throw NPE. */
  public void testHashCode() {
    new TestRecord().hashCode();
    SpecificData.get().hashCode(null, TestRecord.SCHEMA$);
  }

  @Test
  /** Make sure that even with nulls, toString() doesn't throw NPE. */
  public void testToString() {
    new TestRecord().toString();
  }

  private static class X {
    public Map<String, String> map;
  }

  @Test
  public void testGetMapSchema() throws Exception {
    SpecificData.get().getSchema(X.class.getField("map").getGenericType());
  }

  @Test
  /** Test nesting of specific data within generic. */
  public void testSpecificWithinGeneric() throws Exception {
    // define a record with a field that's a generated TestRecord
    Schema schema = Schema.createRecord("Foo", "", "x.y.z", false);
    List<Schema.Field> fields = new ArrayList<>();
    fields.add(new Schema.Field("f", TestRecord.SCHEMA$, "", null));
    schema.setFields(fields);

    // create a generic instance of this record
    TestRecord nested = new TestRecord();
    nested.setName("foo");
    nested.setKind(Kind.BAR);
    nested.setHash(new MD5(new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 }));
    GenericData.Record record = new GenericData.Record(schema);
    record.put("f", nested);

    // test that this instance can be written & re-read
    TestSchema.checkBinary(schema, record, new SpecificDatumWriter<>(), new SpecificDatumReader<>());

    TestSchema.checkDirectBinary(schema, record, new SpecificDatumWriter<>(), new SpecificDatumReader<>());

    TestSchema.checkBlockingBinary(schema, record, new SpecificDatumWriter<>(), new SpecificDatumReader<>());
  }

  @Test
  public void testConvertGenericToSpecific() {
    GenericRecord generic = new GenericData.Record(TestRecord.SCHEMA$);
    generic.put("name", "foo");
    generic.put("kind", new GenericData.EnumSymbol(Kind.SCHEMA$, "BAR"));
    generic.put("hash",
        new GenericData.Fixed(MD5.SCHEMA$, new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 }));
    TestRecord specific = (TestRecord) SpecificData.get().deepCopy(TestRecord.SCHEMA$, generic);
  }

  @Test
  public void testGetClassSchema() throws Exception {
    Assert.assertEquals(TestRecord.getClassSchema(), TestRecord.SCHEMA$);
    Assert.assertEquals(MD5.getClassSchema(), MD5.SCHEMA$);
    Assert.assertEquals(Kind.getClassSchema(), Kind.SCHEMA$);
  }

  @Test
  public void testSpecificRecordToString() throws IOException {
    FooBarSpecificRecord foo = FooBarSpecificRecord.newBuilder().setId(123).setName("foo")
        .setNicknames(Collections.singletonList("bar")).setRelatedids(Arrays.asList(1, 2, 3)).setTypeEnum(TypeEnum.c)
        .build();

    String json = foo.toString();
    JsonFactory factory = new JsonFactory();
    JsonParser parser = factory.createParser(json);
    ObjectMapper mapper = new ObjectMapper();

    // will throw exception if string is not parsable json
    mapper.readTree(parser);
  }

  @Test
  public void testExternalizeable() throws Exception {
    TestRecord before = new TestRecord();
    before.setName("foo");
    before.setKind(Kind.BAR);
    before.setHash(new MD5(new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 }));
    ByteArrayOutputStream bytes = new ByteArrayOutputStream();
    ObjectOutputStream out = new ObjectOutputStream(bytes);
    out.writeObject(before);
    out.close();

    ObjectInputStream in = new ObjectInputStream(new ByteArrayInputStream(bytes.toByteArray()));
    TestRecord after = (TestRecord) in.readObject();

    Assert.assertEquals(before, after);

  }

  @Test
  public void testReservedEnumSymbol() throws Exception {
    Assert.assertEquals(Reserved.default$, SpecificData.get().createEnum("default", Reserved.SCHEMA$));
  }

}
