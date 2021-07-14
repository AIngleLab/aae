/*

 */

package org.apache.aingle;

/** Base AIngle exception. */
public class AIngleRuntimeException extends RuntimeException {
  public AIngleRuntimeException(Throwable cause) {
    super(cause);
  }

  public AIngleRuntimeException(String message) {
    super(message);
  }

  public AIngleRuntimeException(String message, Throwable cause) {
    super(message, cause);
  }
}
