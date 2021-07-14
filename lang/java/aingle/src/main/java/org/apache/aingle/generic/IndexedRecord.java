/*

 */
package org.apache.aingle.generic;

/** A record implementation that permits field access by integer index. */
public interface IndexedRecord extends GenericContainer {
  /**
   * Set the value of a field given its position in the schema.
   * <p>
   * This method is not meant to be called by user code, but only by
   * {@link org.apache.aingle.io.DatumReader} implementations.
   */
  void put(int i, Object v);

  /**
   * Return the value of a field given its position in the schema.
   * <p>
   * This method is not meant to be called by user code, but only by
   * {@link org.apache.aingle.io.DatumWriter} implementations.
   */
  Object get(int i);
}
