/*

 */
package org.apache.aingle.data;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Field;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.IndexedRecord;

import java.io.IOException;
import java.util.Arrays;

/** Abstract base class for RecordBuilder implementations. Not thread-safe. */
public abstract class RecordBuilderBase<T extends IndexedRecord> implements RecordBuilder<T> {
  private final Schema schema;
  private final Field[] fields;
  private final boolean[] fieldSetFlags;
  private final GenericData data;

  protected final Schema schema() {
    return schema;
  }

  protected final Field[] fields() {
    return fields;
  }

  protected final boolean[] fieldSetFlags() {
    return fieldSetFlags;
  }

  protected final GenericData data() {
    return data;
  }

  /**
   * Creates a RecordBuilderBase for building records of the given type.
   * 
   * @param schema the schema associated with the record class.
   */
  protected RecordBuilderBase(Schema schema, GenericData data) {
    this.schema = schema;
    this.data = data;
    fields = schema.getFields().toArray(new Field[0]);
    fieldSetFlags = new boolean[fields.length];
  }

  /**
   * RecordBuilderBase copy constructor. Makes a deep copy of the values in the
   * other builder.
   * 
   * @param other RecordBuilderBase instance to copy.
   */
  protected RecordBuilderBase(RecordBuilderBase<T> other, GenericData data) {
    this.schema = other.schema;
    this.data = data;
    fields = schema.getFields().toArray(new Field[0]);
    fieldSetFlags = Arrays.copyOf(other.fieldSetFlags, other.fieldSetFlags.length);
  }

  /**
   * Validates that a particular value for a given field is valid according to the
   * following algorithm: 1. If the value is not null, or the field type is null,
   * or the field type is a union which accepts nulls, returns. 2. Else, if the
   * field has a default value, returns. 3. Otherwise throws AIngleRuntimeException.
   * 
   * @param field the field to validate.
   * @param value the value to validate.
   * @throws AIngleRuntimeException if value is null and the given field does not
   *                              accept null values.
   */
  protected void validate(Field field, Object value) {
    if (!isValidValue(field, value) && field.defaultVal() == null) {
      throw new AIngleRuntimeException("Field " + field + " does not accept null values");
    }
  }

  /**
   * Tests whether a value is valid for a specified field.
   * 
   * @param f     the field for which to test the value.
   * @param value the value to test.
   * @return true if the value is valid for the given field; false otherwise.
   */
  protected static boolean isValidValue(Field f, Object value) {
    if (value != null) {
      return true;
    }

    Schema schema = f.schema();
    Type type = schema.getType();

    // If the type is null, any value is valid
    if (type == Type.NULL) {
      return true;
    }

    // If the type is a union that allows nulls, any value is valid
    if (type == Type.UNION) {
      for (Schema s : schema.getTypes()) {
        if (s.getType() == Type.NULL) {
          return true;
        }
      }
    }

    // The value is null but the type does not allow nulls
    return false;
  }

  /**
   * Gets the default value of the given field, if any.
   * 
   * @param field the field whose default value should be retrieved.
   * @return the default value associated with the given field, or null if none is
   *         specified in the schema.
   * @throws IOException
   */
  @SuppressWarnings({ "rawtypes", "unchecked" })
  protected Object defaultValue(Field field) throws IOException {
    return data.deepCopy(field.schema(), data.getDefaultValue(field));
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = prime * result + Arrays.hashCode(fieldSetFlags);
    result = prime * result + ((schema == null) ? 0 : schema.hashCode());
    return result;
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj)
      return true;
    if (obj == null)
      return false;
    if (getClass() != obj.getClass())
      return false;
    @SuppressWarnings("rawtypes")
    RecordBuilderBase other = (RecordBuilderBase) obj;
    if (!Arrays.equals(fieldSetFlags, other.fieldSetFlags))
      return false;
    if (schema == null) {
      return other.schema == null;
    } else {
      return schema.equals(other.schema);
    }
  }
}
