/*

 */
package org.apache.aingle.data;

/** Interface for error builders */
public interface ErrorBuilder<T> extends RecordBuilder<T> {

  /** Gets the value */
  Object getValue();

  /** Sets the value */
  ErrorBuilder<T> setValue(Object value);

  /** Checks whether the value has been set */
  boolean hasValue();

  /** Clears the value */
  ErrorBuilder<T> clearValue();

  /** Gets the error cause */
  Throwable getCause();

  /** Sets the error cause */
  ErrorBuilder<T> setCause(Throwable cause);

  /** Checks whether the cause has been set */
  boolean hasCause();

  /** Clears the cause */
  ErrorBuilder<T> clearCause();

}
