/**

 */
package org.apache.aingle.util.internal;

import java.io.IOException;

import org.apache.aingle.JsonProperties;
import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Field;
import org.apache.aingle.Schema.Field.Order;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.io.JsonEncoder;
import org.apache.aingle.io.parsing.ResolvingGrammarGenerator;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;

public class Accessor {
  public abstract static class JsonPropertiesAccessor {
    protected abstract void addProp(JsonProperties props, String name, JsonNode value);
  }

  public abstract static class FieldAccessor {
    protected abstract JsonNode defaultValue(Field field);

    protected abstract Field createField(String name, Schema schema, String doc, JsonNode defaultValue,
        boolean validate, Order order);

    protected abstract Field createField(String name, Schema schema, String doc, JsonNode defaultValue);
  }

  public abstract static class ResolvingGrammarGeneratorAccessor {
    protected abstract void encode(Encoder e, Schema s, JsonNode n) throws IOException;
  }

  public abstract static class EncoderFactoryAccessor {
    protected abstract JsonEncoder jsonEncoder(EncoderFactory factory, Schema schema, JsonGenerator gen)
        throws IOException;
  }

  private static volatile JsonPropertiesAccessor jsonPropertiesAccessor;

  private static volatile FieldAccessor fieldAccessor;

  private static volatile ResolvingGrammarGeneratorAccessor resolvingGrammarGeneratorAccessor;

  public static void setAccessor(JsonPropertiesAccessor accessor) {
    if (jsonPropertiesAccessor != null)
      throw new IllegalStateException("JsonPropertiesAccessor already initialized");
    jsonPropertiesAccessor = accessor;
  }

  public static void setAccessor(FieldAccessor accessor) {
    if (fieldAccessor != null)
      throw new IllegalStateException("FieldAccessor already initialized");
    fieldAccessor = accessor;
  }

  private static FieldAccessor fieldAccessor() {
    if (fieldAccessor == null)
      ensureLoaded(Field.class);
    return fieldAccessor;
  }

  public static void setAccessor(ResolvingGrammarGeneratorAccessor accessor) {
    if (resolvingGrammarGeneratorAccessor != null)
      throw new IllegalStateException("ResolvingGrammarGeneratorAccessor already initialized");
    resolvingGrammarGeneratorAccessor = accessor;
  }

  private static ResolvingGrammarGeneratorAccessor resolvingGrammarGeneratorAccessor() {
    if (resolvingGrammarGeneratorAccessor == null)
      ensureLoaded(ResolvingGrammarGenerator.class);
    return resolvingGrammarGeneratorAccessor;
  }

  private static void ensureLoaded(Class<?> c) {
    try {
      Class.forName(c.getName());
    } catch (ClassNotFoundException e) {
      // Shall never happen as the class is specified by its Class instance
    }
  }

  public static void addProp(JsonProperties props, String name, JsonNode value) {
    jsonPropertiesAccessor.addProp(props, name, value);
  }

  public static JsonNode defaultValue(Field field) {
    return fieldAccessor.defaultValue(field);
  }

  public static void encode(Encoder e, Schema s, JsonNode n) throws IOException {
    resolvingGrammarGeneratorAccessor().encode(e, s, n);
  }

  public static Field createField(String name, Schema schema, String doc, JsonNode defaultValue, boolean validate,
      Order order) {
    return fieldAccessor().createField(name, schema, doc, defaultValue, validate, order);
  }

  public static Field createField(String name, Schema schema, String doc, JsonNode defaultValue) {
    return fieldAccessor().createField(name, schema, doc, defaultValue);
  }

}
