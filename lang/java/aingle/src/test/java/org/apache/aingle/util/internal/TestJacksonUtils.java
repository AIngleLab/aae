/*

 */
package org.apache.aingle.util.internal;

import static org.apache.aingle.util.internal.JacksonUtils.toJsonNode;
import static org.apache.aingle.util.internal.JacksonUtils.toObject;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.BigIntegerNode;
import com.fasterxml.jackson.databind.node.BooleanNode;
import com.fasterxml.jackson.databind.node.DecimalNode;
import com.fasterxml.jackson.databind.node.DoubleNode;
import com.fasterxml.jackson.databind.node.FloatNode;
import com.fasterxml.jackson.databind.node.IntNode;
import com.fasterxml.jackson.databind.node.JsonNodeFactory;
import com.fasterxml.jackson.databind.node.LongNode;
import com.fasterxml.jackson.databind.node.NullNode;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.fasterxml.jackson.databind.node.TextNode;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.Collections;
import org.apache.aingle.JsonProperties;
import org.apache.aingle.Schema;
import org.apache.aingle.SchemaBuilder;
import org.junit.Test;

public class TestJacksonUtils {

  enum Direction {
    UP, DOWN;
  }

  @Test
  public void testToJsonNode() {
    assertEquals(null, toJsonNode(null));
    assertEquals(NullNode.getInstance(), toJsonNode(JsonProperties.NULL_VALUE));
    assertEquals(BooleanNode.TRUE, toJsonNode(true));
    assertEquals(IntNode.valueOf(1), toJsonNode(1));
    assertEquals(LongNode.valueOf(2), toJsonNode(2L));
    assertEquals(FloatNode.valueOf(1.0f), toJsonNode(1.0f));
    assertEquals(DoubleNode.valueOf(2.0), toJsonNode(2.0));
    assertEquals(TextNode.valueOf("\u0001\u0002"), toJsonNode(new byte[] { 1, 2 }));
    assertEquals(TextNode.valueOf("a"), toJsonNode("a"));
    assertEquals(TextNode.valueOf("UP"), toJsonNode(Direction.UP));
    assertEquals(BigIntegerNode.valueOf(BigInteger.ONE), toJsonNode(BigInteger.ONE));
    assertEquals(DecimalNode.valueOf(BigDecimal.ONE), toJsonNode(BigDecimal.ONE));

    ArrayNode an = JsonNodeFactory.instance.arrayNode();
    an.add(1);
    assertEquals(an, toJsonNode(Collections.singletonList(1)));

    ObjectNode on = JsonNodeFactory.instance.objectNode();
    on.put("a", 1);
    assertEquals(on, toJsonNode(Collections.singletonMap("a", 1)));
  }

  @Test
  public void testToObject() {
    assertEquals(null, toObject(null));
    assertEquals(JsonProperties.NULL_VALUE, toObject(NullNode.getInstance()));
    assertEquals(true, toObject(BooleanNode.TRUE));
    assertEquals(1, toObject(IntNode.valueOf(1)));
    assertEquals(2L, toObject(IntNode.valueOf(2), Schema.create(Schema.Type.LONG)));
    assertEquals(1.0f, toObject(DoubleNode.valueOf(1.0), Schema.create(Schema.Type.FLOAT)));
    assertEquals(2.0, toObject(DoubleNode.valueOf(2.0)));
    assertEquals(TextNode.valueOf("\u0001\u0002"), toJsonNode(new byte[] { 1, 2 }));
    assertArrayEquals(new byte[] { 1, 2 },
        (byte[]) toObject(TextNode.valueOf("\u0001\u0002"), Schema.create(Schema.Type.BYTES)));
    assertEquals("a", toObject(TextNode.valueOf("a")));
    assertEquals("UP", toObject(TextNode.valueOf("UP"), SchemaBuilder.enumeration("Direction").symbols("UP", "DOWN")));

    ArrayNode an = JsonNodeFactory.instance.arrayNode();
    an.add(1);
    assertEquals(Collections.singletonList(1), toObject(an));

    ObjectNode on = JsonNodeFactory.instance.objectNode();
    on.put("a", 1);
    assertEquals(Collections.singletonMap("a", 1), toObject(on));
    assertEquals(Collections.singletonMap("a", 1L),
        toObject(on, SchemaBuilder.record("r").fields().requiredLong("a").endRecord()));

    assertEquals(JsonProperties.NULL_VALUE,
        toObject(NullNode.getInstance(), SchemaBuilder.unionOf().nullType().and().intType().endUnion()));

    assertEquals("a", toObject(TextNode.valueOf("a"), SchemaBuilder.unionOf().stringType().and().intType().endUnion()));
  }

}
