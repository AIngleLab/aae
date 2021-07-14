/*

 */

package org.apache.aingle;

/** Thrown when an illegal type is used. */
public class AIngleTypeException extends AIngleRuntimeException {
  public AIngleTypeException(String message) {
    super(message);
  }

  public AIngleTypeException(String message, Throwable cause) {
    super(message, cause);
  }
}
