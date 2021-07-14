/*

 */

package org.apache.trevni;

/** Base runtime exception thrown by Trevni. */
public class TrevniRuntimeException extends RuntimeException {
  public TrevniRuntimeException(Throwable cause) {
    super(cause);
  }

  public TrevniRuntimeException(String message) {
    super(message);
  }

  public TrevniRuntimeException(String message, Throwable cause) {
    super(message, cause);
  }
}
