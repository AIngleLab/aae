/*

 */
package org.apache.aingle.generic;

/**
 * A generic instance of a record schema. Fields are accessible by name as well
 * as by index.
 */
public interface GenericRecord extends IndexedRecord {
  /** Set the value of a field given its name. */
  void put(String key, Object v);

  /** Return the value of a field given its name. */
  Object get(String key);

  /** Return true if record has field with name: key */
  default boolean hasField(String key) {
    return getSchema().getField(key) != null;
  }
}
