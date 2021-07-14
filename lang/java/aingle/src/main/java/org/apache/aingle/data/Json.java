/*

 */
package org.apache.aingle.data;

import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

import org.apache.aingle.util.internal.JacksonUtils;
import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.JsonNodeFactory;
import com.fasterxml.jackson.databind.node.LongNode;
import com.fasterxml.jackson.databind.node.DoubleNode;
import com.fasterxml.jackson.databind.node.TextNode;
import com.fasterxml.jackson.databind.node.BooleanNode;
import com.fasterxml.jackson.databind.node.NullNode;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

import org.apache.aingle.Schema;
import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.ResolvingDecoder;

/** Utilities for reading and writing arbitrary Json data in AIngle format. */
public class Json {
  private Json() {
  } // singleton: no public ctor

  static final JsonFactory FACTORY = new JsonFactory();
  static final ObjectMapper MAPPER = new ObjectMapper(FACTORY);

  /** The schema for Json data. */
  public static final Schema SCHEMA;
  static {
    try {
      try (InputStream in = Json.class.getResourceAsStream("/org/apache/aingle/data/Json.ain")) {
        SCHEMA = new Schema.Parser().parse(in);
      }
    } catch (IOException e) {
      throw new AIngleRuntimeException(e);
    }
  }

  /**
   * {@link DatumWriter} for arbitrary Json data using the object model described
   * in {@link org.apache.aingle.JsonProperties}.
   */
  public static class ObjectWriter implements DatumWriter<Object> {

    @Override
    public void setSchema(Schema schema) {
      if (!SCHEMA.equals(schema))
        throw new RuntimeException("Not the Json schema: " + schema);
    }

    @Override
    public void write(Object datum, Encoder out) throws IOException {
      Json.writeObject(datum, out);
    }
  }

  /**
   * {@link DatumReader} for arbitrary Json data using the object model described
   * in {@link org.apache.aingle.JsonProperties}.
   */
  public static class ObjectReader implements DatumReader<Object> {
    private Schema written;
    private ResolvingDecoder resolver;

    @Override
    public void setSchema(Schema schema) {
      this.written = SCHEMA.equals(written) ? null : schema;
    }

    @Override
    public Object read(Object reuse, Decoder in) throws IOException {
      if (written == null) // same schema
        return Json.readObject(in);

      // use a resolver to adapt alternate version of Json schema
      if (resolver == null)
        resolver = DecoderFactory.get().resolvingDecoder(written, SCHEMA, null);
      resolver.configure(in);
      Object result = Json.readObject(resolver);
      resolver.drain();
      return result;
    }
  }

  /**
   * Parses a JSON string and converts it to the object model described in
   * {@link org.apache.aingle.JsonProperties}.
   */
  public static Object parseJson(String s) {
    try {
      return JacksonUtils.toObject(MAPPER.readTree(FACTORY.createParser(s)));
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
  }

  /**
   * Converts an instance of the object model described in
   * {@link org.apache.aingle.JsonProperties} to a JSON string.
   */
  public static String toString(Object datum) {
    return JacksonUtils.toJsonNode(datum).toString();
  }

  /** Note: this enum must be kept aligned with the union in Json.ain. */
  private enum JsonType {
    LONG, DOUBLE, STRING, BOOLEAN, NULL, ARRAY, OBJECT
  }

  /**
   * Write Json data as AIngle data.
   */
  private static void write(JsonNode node, Encoder out) throws IOException {
    switch (node.asToken()) {
    case VALUE_NUMBER_INT:
      out.writeIndex(JsonType.LONG.ordinal());
      out.writeLong(node.longValue());
      break;
    case VALUE_NUMBER_FLOAT:
      out.writeIndex(JsonType.DOUBLE.ordinal());
      out.writeDouble(node.doubleValue());
      break;
    case VALUE_STRING:
      out.writeIndex(JsonType.STRING.ordinal());
      out.writeString(node.textValue());
      break;
    case VALUE_TRUE:
      out.writeIndex(JsonType.BOOLEAN.ordinal());
      out.writeBoolean(true);
      break;
    case VALUE_FALSE:
      out.writeIndex(JsonType.BOOLEAN.ordinal());
      out.writeBoolean(false);
      break;
    case VALUE_NULL:
      out.writeIndex(JsonType.NULL.ordinal());
      out.writeNull();
      break;
    case START_ARRAY:
      out.writeIndex(JsonType.ARRAY.ordinal());
      out.writeArrayStart();
      out.setItemCount(node.size());
      for (JsonNode element : node) {
        out.startItem();
        write(element, out);
      }
      out.writeArrayEnd();
      break;
    case START_OBJECT:
      out.writeIndex(JsonType.OBJECT.ordinal());
      out.writeMapStart();
      out.setItemCount(node.size());
      Iterator<String> i = node.fieldNames();
      while (i.hasNext()) {
        out.startItem();
        String name = i.next();
        out.writeString(name);
        write(node.get(name), out);
      }
      out.writeMapEnd();
      break;
    default:
      throw new AIngleRuntimeException(node.asToken() + " unexpected: " + node);
    }
  }

  /**
   * Read Json data from AIngle data.
   */
  private static JsonNode read(Decoder in) throws IOException {
    switch (JsonType.values()[in.readIndex()]) {
    case LONG:
      return new LongNode(in.readLong());
    case DOUBLE:
      return new DoubleNode(in.readDouble());
    case STRING:
      return new TextNode(in.readString());
    case BOOLEAN:
      return in.readBoolean() ? BooleanNode.TRUE : BooleanNode.FALSE;
    case NULL:
      in.readNull();
      return NullNode.getInstance();
    case ARRAY:
      ArrayNode array = JsonNodeFactory.instance.arrayNode();
      for (long l = in.readArrayStart(); l > 0; l = in.arrayNext())
        for (long i = 0; i < l; i++)
          array.add(read(in));
      return array;
    case OBJECT:
      ObjectNode object = JsonNodeFactory.instance.objectNode();
      for (long l = in.readMapStart(); l > 0; l = in.mapNext())
        for (long i = 0; i < l; i++)
          object.set(in.readString(), read(in));
      return object;
    default:
      throw new AIngleRuntimeException("Unexpected Json node type");
    }
  }

  private static void writeObject(Object datum, Encoder out) throws IOException {
    write(JacksonUtils.toJsonNode(datum), out);
  }

  private static Object readObject(Decoder in) throws IOException {
    return JacksonUtils.toObject(read(in));
  }

}
