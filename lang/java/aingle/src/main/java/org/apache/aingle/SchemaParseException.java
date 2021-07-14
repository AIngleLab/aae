/*

 */

package org.apache.aingle;

/** Thrown for errors parsing schemas and protocols. */
public class SchemaParseException extends AIngleRuntimeException {
  public SchemaParseException(Throwable cause) {
    super(cause);
  }

  public SchemaParseException(String message) {
    super(message);
  }
}
