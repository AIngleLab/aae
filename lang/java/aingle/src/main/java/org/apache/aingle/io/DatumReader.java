/*

 */
package org.apache.aingle.io;

import java.io.IOException;

import org.apache.aingle.Schema;

/**
 * Read data of a schema.
 * <p>
 * Determines the in-memory data representation.
 */
public interface DatumReader<D> {

  /** Set the writer's schema. */
  void setSchema(Schema schema);

  /**
   * Read a datum. Traverse the schema, depth-first, reading all leaf values in
   * the schema into a datum that is returned. If the provided datum is non-null
   * it may be reused and returned.
   */
  D read(D reuse, Decoder in) throws IOException;

}
