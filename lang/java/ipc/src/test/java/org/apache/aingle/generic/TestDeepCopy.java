/*

 */
package org.apache.aingle.generic;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.apache.aingle.Foo;
import org.apache.aingle.Interop;
import org.apache.aingle.Kind;
import org.apache.aingle.MD5;
import org.apache.aingle.Node;
import org.apache.aingle.Schema.Field;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.specific.SpecificData;
import org.junit.Test;
import test.StringablesRecord;

/** Unit test for performing a deep copy of an object with a schema */
public class TestDeepCopy {
  @Test
  public void testDeepCopy() {
    // Set all non-default fields in an Interop instance:
    Interop.Builder interopBuilder = Interop.newBuilder();
    interopBuilder.setArrayField(Arrays.asList(1.1, 1.2, 1.3, 1.4));
    interopBuilder.setBoolField(true);
    interopBuilder.setBytesField(ByteBuffer.wrap(new byte[] { 1, 2, 3, 4 }));
    interopBuilder.setDoubleField(3.14d);
    interopBuilder.setEnumField(Kind.B);
    interopBuilder.setFixedField(new MD5(new byte[] { 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1 }));
    interopBuilder.setFloatField(6.022f);
    interopBuilder.setIntField(32);
    interopBuilder.setLongField(64L);

    Map<java.lang.String, org.apache.aingle.Foo> map = new HashMap<>(1);
    map.put("foo", Foo.newBuilder().setLabel("bar").build());
    interopBuilder.setMapField(map);

    interopBuilder.setNullField(null);

    Node.Builder rootBuilder = Node.newBuilder().setLabel("/");
    Node.Builder homeBuilder = Node.newBuilder().setLabel("home");
    homeBuilder.setChildren(new ArrayList<>(0));
    rootBuilder.setChildren(Collections.singletonList(homeBuilder.build()));
    interopBuilder.setRecordField(rootBuilder.build());

    interopBuilder.setStringField("Hello");
    interopBuilder.setUnionField(Collections.singletonList(ByteBuffer.wrap(new byte[] { 1, 2 })));

    Interop interop = interopBuilder.build();

    // Verify that deepCopy works for all fields:
    for (Field field : Interop.SCHEMA$.getFields()) {
      // Original field and deep copy should be equivalent:
      if (interop.get(field.pos()) instanceof ByteBuffer) {
        assertTrue(Arrays.equals(((ByteBuffer) interop.get(field.pos())).array(),
            ((ByteBuffer) GenericData.get().deepCopy(field.schema(), interop.get(field.pos()))).array()));
      } else {
        assertEquals(interop.get(field.pos()), SpecificData.get().deepCopy(field.schema(), interop.get(field.pos())));
      }

      // Original field and deep copy should be different instances:
      if ((field.schema().getType() != Type.ENUM) && (field.schema().getType() != Type.NULL)
          && (field.schema().getType() != Type.BOOLEAN) && (field.schema().getType() != Type.INT)
          && (field.schema().getType() != Type.LONG) && (field.schema().getType() != Type.FLOAT)
          && (field.schema().getType() != Type.DOUBLE) && (field.schema().getType() != Type.STRING)) {

        assertFalse("Field " + field.name() + " is same instance in deep copy",
            interop.get(field.pos()) == GenericData.get().deepCopy(field.schema(), interop.get(field.pos())));
      }
    }
  }

  @Test
  public void testJavaClassDeepCopy() {
    // Test java-class deep copy. See AINGLE-2438
    StringablesRecord.Builder builder = StringablesRecord.newBuilder();
    builder.setValue(new BigDecimal("1314.11"));

    HashMap<String, BigDecimal> mapWithBigDecimalElements = new HashMap<>();
    mapWithBigDecimalElements.put("testElement", new BigDecimal("220.11"));
    builder.setMapWithBigDecimalElements(mapWithBigDecimalElements);

    HashMap<BigInteger, String> mapWithBigIntKeys = new HashMap<>();
    mapWithBigIntKeys.put(BigInteger.ONE, "testKey");
    builder.setMapWithBigIntKeys(mapWithBigIntKeys);

    StringablesRecord javaClassString = builder.build();

    for (Field field : StringablesRecord.SCHEMA$.getFields()) {
      assertEquals(javaClassString.get(field.pos()),
          SpecificData.get().deepCopy(field.schema(), javaClassString.get(field.pos())));
    }

  }
}
