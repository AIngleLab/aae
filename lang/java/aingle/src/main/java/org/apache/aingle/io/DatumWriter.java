/*

 */
package org.apache.aingle.io;

import java.io.IOException;

import org.apache.aingle.Schema;

/**
 * Write data of a schema.
 * <p>
 * Implemented for different in-memory data representations.
 */
public interface DatumWriter<D> {

  /** Set the schema. */
  void setSchema(Schema schema);

  /**
   * Write a datum. Traverse the schema, depth first, writing each leaf value in
   * the schema from the datum to the output.
   */
  void write(D datum, Encoder out) throws IOException;
}
