/*

 */

package org.apache.aingle;

/** Thrown for errors building schemas. */
public class SchemaBuilderException extends AIngleRuntimeException {
  public SchemaBuilderException(Throwable cause) {
    super(cause);
  }

  public SchemaBuilderException(String message) {
    super(message);
  }
}
