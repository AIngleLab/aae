/*

 */

package org.apache.aingle.reflect;

import org.apache.aingle.AIngleTypeException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.util.internal.JacksonUtils;
import org.junit.Test;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import static org.hamcrest.Matchers.*;
import static org.junit.Assert.*;

public class TestReflectData {
  @Test
  @SuppressWarnings("unchecked")
  public void testWeakSchemaCaching() throws Exception {
    int numSchemas = 1000000;
    for (int i = 0; i < numSchemas; i++) {
      // Create schema
      Schema schema = Schema.createRecord("schema", null, null, false);
      schema.setFields(Collections.emptyList());

      ReflectData.get().getRecordState(new Object(), schema);
    }

    // Reflect the number of schemas currently in the cache
    ReflectData.ClassAccessorData classData = ReflectData.ACCESSOR_CACHE.get(Object.class);

    System.gc(); // Not guaranteed, but seems to be reliable enough

    assertThat("ReflectData cache should release references", classData.bySchema.size(), lessThan(numSchemas));
  }

  @Test
  public void testGenericProtocol() {
    Protocol protocol = ReflectData.get().getProtocol(FooBarProtocol.class);
    Schema recordSchema = ReflectData.get().getSchema(FooBarReflectiveRecord.class);

    assertThat(protocol.getTypes(), contains(recordSchema));

    assertThat(protocol.getMessages().keySet(), containsInAnyOrder("store", "findById", "exists"));

    Schema.Field storeArgument = protocol.getMessages().get("store").getRequest().getFields().get(0);
    assertThat(storeArgument.schema(), equalTo(recordSchema));

    Schema.Field findByIdArgument = protocol.getMessages().get("findById").getRequest().getFields().get(0);
    assertThat(findByIdArgument.schema(), equalTo(Schema.create(Schema.Type.STRING)));

    Schema findByIdResponse = protocol.getMessages().get("findById").getResponse();
    assertThat(findByIdResponse, equalTo(recordSchema));

    Schema.Field existsArgument = protocol.getMessages().get("exists").getRequest().getFields().get(0);
    assertThat(existsArgument.schema(), equalTo(Schema.create(Schema.Type.STRING)));
  }

  private interface CrudProtocol<R, I> extends OtherProtocol<I> {
    void store(R record);

    R findById(I id);
  }

  private interface OtherProtocol<G> {
    boolean exists(G id);
  }

  private interface FooBarProtocol extends OtherProtocol<String>, CrudProtocol<FooBarReflectiveRecord, String> {
  }

  private static class FooBarReflectiveRecord {
    private String bar;
    private int baz;
  }

  static class User {
    public String first = "AIngle";
    public String last = "Apache";
  }

  static class Meta {
    public int f1 = 55;
    public int f4;
    public String f2 = "a-string";
    public List<String> f3 = Arrays.asList("one", "two", "three");
    // public User usr = new User();
  }

  @Test
  public void testCreateSchemaDefaultValue() {
    Meta meta = new Meta();
    validateSchema(meta);

    meta.f4 = 0x1987;
    validateSchema(meta);
  }

  private void validateSchema(Meta meta) {
    Schema schema = new ReflectData().setDefaultsGenerated(true).setDefaultGeneratedValue(Meta.class, meta)
        .getSchema(Meta.class);

    final String schemaString = schema.toString(true);

    Schema.Parser parser = new Schema.Parser();
    Schema cloneSchema = parser.parse(schemaString);

    Map testCases = JacksonUtils.objectToMap(meta);

    for (Schema.Field field : cloneSchema.getFields()) {
      assertEquals("Invalid field " + field.name(), field.defaultVal(), testCases.get(field.name()));
    }
  }

  public class Definition {
    public Map<String, String> tokens;
  }

  @Test(expected = AIngleTypeException.class)
  public void testNonStaticInnerClasses() {
    ReflectData.get().getSchema(Definition.class);
  }

  @Test
  public void testStaticInnerClasses() {
    ReflectData.get().getSchema(Meta.class);
  }
}
