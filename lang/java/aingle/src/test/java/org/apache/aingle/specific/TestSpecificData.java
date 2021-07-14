/*

 */

package org.apache.aingle.specific;

import static org.junit.Assert.*;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Field;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.junit.Before;
import org.junit.Test;

/*
 * If integerClass is primitive, reflection to find method will
 * result in a NoSuchMethodException in the case of a UNION schema
 */
public class TestSpecificData {

  private Class<?> intClass;
  private Class<?> integerClass;

  @Before
  public void setUp() {
    Schema intSchema = Schema.create(Type.INT);
    intClass = SpecificData.get().getClass(intSchema);
    Schema nullSchema = Schema.create(Type.NULL);
    Schema nullIntUnionSchema = Schema.createUnion(Arrays.asList(nullSchema, intSchema));
    integerClass = SpecificData.get().getClass(nullIntUnionSchema);
  }

  @Test
  public void testClassTypes() {
    assertTrue(intClass.isPrimitive());
    assertFalse(integerClass.isPrimitive());
  }

  @Test
  public void testPrimitiveParam() throws Exception {
    assertNotNull(Reflection.class.getMethod("primitive", intClass));
  }

  @Test(expected = NoSuchMethodException.class)
  public void testPrimitiveParamError() throws Exception {
    Reflection.class.getMethod("primitiveWrapper", intClass);
  }

  @Test
  public void testPrimitiveWrapperParam() throws Exception {
    assertNotNull(Reflection.class.getMethod("primitiveWrapper", integerClass));
  }

  @Test(expected = NoSuchMethodException.class)
  public void testPrimitiveWrapperParamError() throws Exception {
    Reflection.class.getMethod("primitive", integerClass);
  }

  static class Reflection {
    public void primitive(int i) {
    }

    public void primitiveWrapper(Integer i) {
    }
  }

  public static class TestRecord extends SpecificRecordBase {
    private static final Schema SCHEMA = Schema.createRecord("TestRecord", null, null, false);
    static {
      List<Field> fields = new ArrayList<>();
      fields.add(new Field("x", Schema.create(Type.INT), null, null));
      Schema stringSchema = Schema.create(Type.STRING);
      GenericData.setStringType(stringSchema, GenericData.StringType.String);
      fields.add(new Field("y", stringSchema, null, null));
      SCHEMA.setFields(fields);
    }
    private int x;
    private String y;

    @Override
    public void put(int i, Object v) {
      switch (i) {
      case 0:
        x = (Integer) v;
        break;
      case 1:
        y = (String) v;
        break;
      default:
        throw new RuntimeException();
      }
    }

    @Override
    public Object get(int i) {
      switch (i) {
      case 0:
        return x;
      case 1:
        return y;
      }
      throw new RuntimeException();
    }

    @Override
    public Schema getSchema() {
      return SCHEMA;
    }

  }

  @Test
  public void testSpecificRecordBase() {
    final TestRecord record = new TestRecord();
    record.put("x", 1);
    record.put("y", "str");
    assertEquals(1, record.get("x"));
    assertEquals("str", record.get("y"));
  }

  @Test
  public void testExternalizeable() throws Exception {
    final TestRecord before = new TestRecord();
    before.put("x", 1);
    before.put("y", "str");
    ByteArrayOutputStream bytes = new ByteArrayOutputStream();
    ObjectOutputStream out = new ObjectOutputStream(bytes);
    out.writeObject(before);
    out.close();

    ObjectInputStream in = new ObjectInputStream(new ByteArrayInputStream(bytes.toByteArray()));
    TestRecord after = (TestRecord) in.readObject();

    assertEquals(before, after);
  }

  /** Tests that non Stringable datum are rejected by specific writers. */
  @Test
  public void testNonStringable() throws Exception {
    final Schema string = Schema.create(Type.STRING);
    final ByteArrayOutputStream baos = new ByteArrayOutputStream();
    final Encoder encoder = EncoderFactory.get().directBinaryEncoder(baos, null);
    final DatumWriter<Object> writer = new SpecificDatumWriter<>(string);
    try {
      writer.write(new Object(), encoder);
      fail("Non stringable object should be rejected.");
    } catch (ClassCastException cce) {
      // Expected error
    }
  }
}
