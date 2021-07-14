/*

 */

package org.apache.aingle;

/** Base class for exceptions thrown to client by server. */
public class AIngleRemoteException extends Exception {
  private Object value;

  protected AIngleRemoteException() {
  }

  public AIngleRemoteException(Throwable value) {
    this(value.toString());
    initCause(value);
  }

  public AIngleRemoteException(Object value) {
    super(value != null ? value.toString() : null);
    this.value = value;
  }

  public AIngleRemoteException(Object value, Throwable cause) {
    super(value != null ? value.toString() : null, cause);
    this.value = value;
  }

  public Object getValue() {
    return value;
  }
}
