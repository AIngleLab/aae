/*

 */
package org.apache.aingle.data;

/** Interface for record builders */
public interface RecordBuilder<T> {
  /**
   * Constructs a new instance using the values set in the RecordBuilder. If a
   * particular value was not set and the schema defines a default value, the
   * default value will be used.
   * 
   * @return a new instance using values set in the RecordBuilder.
   */
  T build();
}
