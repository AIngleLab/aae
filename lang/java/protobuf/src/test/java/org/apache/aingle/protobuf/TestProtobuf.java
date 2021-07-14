/*

 */
package org.apache.aingle.protobuf;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

import org.apache.aingle.Schema;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.specific.SpecificData;
import org.apache.commons.compress.utils.Lists;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

import com.google.protobuf.ByteString;

import org.apache.aingle.protobuf.noopt.Test.Foo;
import org.apache.aingle.protobuf.noopt.Test.A;
import org.apache.aingle.protobuf.noopt.Test.M.N;

public class TestProtobuf {
  @Test
  public void testMessage() throws Exception {

    System.out.println(ProtobufData.get().getSchema(Foo.class).toString(true));
    Foo.Builder builder = Foo.newBuilder();
    builder.setInt32(0);
    builder.setInt64(2);
    builder.setUint32(3);
    builder.setUint64(4);
    builder.setSint32(5);
    builder.setSint64(6);
    builder.setFixed32(7);
    builder.setFixed64(8);
    builder.setSfixed32(9);
    builder.setSfixed64(10);
    builder.setFloat(1.0F);
    builder.setDouble(2.0);
    builder.setBool(true);
    builder.setString("foo");
    builder.setBytes(ByteString.copyFromUtf8("bar"));
    builder.setEnum(A.X);
    builder.addIntArray(27);
    builder.addSyms(A.Y);
    Foo fooInner = builder.build();

    Foo fooInArray = builder.build();
    builder = Foo.newBuilder(fooInArray);
    builder.addFooArray(fooInArray);

    com.google.protobuf.Timestamp ts = com.google.protobuf.Timestamp.newBuilder().setSeconds(1L).setNanos(2).build();
    builder.setTimestamp(ts);

    builder = Foo.newBuilder(fooInner);
    builder.setFoo(fooInner);
    Foo foo = builder.build();

    System.out.println(foo);

    ByteArrayOutputStream bao = new ByteArrayOutputStream();
    ProtobufDatumWriter<Foo> w = new ProtobufDatumWriter<>(Foo.class);
    Encoder e = EncoderFactory.get().binaryEncoder(bao, null);
    w.write(foo, e);
    e.flush();

    Object o = new ProtobufDatumReader<>(Foo.class).read(null,
        DecoderFactory.get().binaryDecoder(new ByteArrayInputStream(bao.toByteArray()), null));

    assertEquals(foo, o);
  }

  @Test
  public void testMessageWithEmptyArray() throws Exception {
    Foo foo = Foo.newBuilder().setInt32(5).setBool(true).build();
    ByteArrayOutputStream bao = new ByteArrayOutputStream();
    ProtobufDatumWriter<Foo> w = new ProtobufDatumWriter<>(Foo.class);
    Encoder e = EncoderFactory.get().binaryEncoder(bao, null);
    w.write(foo, e);
    e.flush();
    Foo o = new ProtobufDatumReader<>(Foo.class).read(null,
        DecoderFactory.get().binaryDecoder(new ByteArrayInputStream(bao.toByteArray()), null));

    assertEquals(foo.getInt32(), o.getInt32());
    assertEquals(foo.getBool(), o.getBool());
    assertEquals(0, o.getFooArrayCount());
  }

  @Test
  public void testEmptyArray() throws Exception {
    Schema s = ProtobufData.get().getSchema(Foo.class);
    assertEquals(s.getField("fooArray").defaultVal(), Lists.newArrayList());
  }

  @Test
  public void testNestedEnum() throws Exception {
    Schema s = ProtobufData.get().getSchema(N.class);
    assertEquals(N.class.getName(), SpecificData.get().getClass(s).getName());
  }

  @Test
  public void testNestedClassNamespace() throws Exception {
    Schema s = ProtobufData.get().getSchema(Foo.class);
    assertEquals(org.apache.aingle.protobuf.noopt.Test.class.getName(), s.getNamespace());
  }

  @Test
  public void testClassNamespaceInMultipleFiles() throws Exception {
    Schema fooSchema = ProtobufData.get().getSchema(org.apache.aingle.protobuf.multiplefiles.Foo.class);
    assertEquals(org.apache.aingle.protobuf.multiplefiles.Foo.class.getPackage().getName(), fooSchema.getNamespace());

    Schema nSchema = ProtobufData.get().getSchema(org.apache.aingle.protobuf.multiplefiles.M.N.class);
    assertEquals(org.apache.aingle.protobuf.multiplefiles.M.class.getName(), nSchema.getNamespace());
  }

  @Test
  public void testGetNonRepeatedSchemaWithLogicalType() throws Exception {
    ProtoConversions.TimestampMillisConversion conversion = new ProtoConversions.TimestampMillisConversion();

    // Don't convert to logical type if conversion isn't set
    ProtobufData instance1 = new ProtobufData();
    Schema s1 = instance1.getSchema(com.google.protobuf.Timestamp.class);
    assertNotEquals(conversion.getRecommendedSchema(), s1);

    // Convert to logical type if conversion is set
    ProtobufData instance2 = new ProtobufData();
    instance2.addLogicalTypeConversion(conversion);
    Schema s2 = instance2.getSchema(com.google.protobuf.Timestamp.class);
    assertEquals(conversion.getRecommendedSchema(), s2);
  }
}
