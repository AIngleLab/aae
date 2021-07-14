/*

 */
package org.apache.aingle.specific;

import java.lang.reflect.Constructor;

import org.apache.aingle.Schema;
import org.apache.aingle.data.ErrorBuilder;
import org.apache.aingle.data.RecordBuilderBase;

/**
 * Abstract base class for specific ErrorBuilder implementations. Not
 * thread-safe.
 */
abstract public class SpecificErrorBuilderBase<T extends SpecificExceptionBase> extends RecordBuilderBase<T>
    implements ErrorBuilder<T> {
  private Constructor<T> errorConstructor;
  private Object value;
  private boolean hasValue;
  private Throwable cause;
  private boolean hasCause;

  /**
   * Creates a SpecificErrorBuilderBase for building errors of the given type.
   * 
   * @param schema the schema associated with the error class.
   */
  protected SpecificErrorBuilderBase(Schema schema) {
    super(schema, SpecificData.get());
  }

  /**
   * Creates a SpecificErrorBuilderBase for building errors of the given type.
   * 
   * @param schema the schema associated with the error class.
   * @param model  the SpecificData instance associated with the error class
   */
  protected SpecificErrorBuilderBase(Schema schema, SpecificData model) {
    super(schema, model);
  }

  /**
   * SpecificErrorBuilderBase copy constructor.
   * 
   * @param other SpecificErrorBuilderBase instance to copy.
   */
  protected SpecificErrorBuilderBase(SpecificErrorBuilderBase<T> other) {
    super(other, SpecificData.get());
    this.errorConstructor = other.errorConstructor;
    this.value = other.value;
    this.hasValue = other.hasValue;
    this.cause = other.cause;
    this.hasCause = other.hasCause;
  }

  /**
   * Creates a SpecificErrorBuilderBase by copying an existing error instance.
   * 
   * @param other the error instance to copy.
   */
  protected SpecificErrorBuilderBase(T other) {
    super(other.getSchema(), SpecificData.get());

    Object otherValue = other.getValue();
    if (otherValue != null) {
      setValue(otherValue);
    }

    Throwable otherCause = other.getCause();
    if (otherCause != null) {
      setCause(otherCause);
    }
  }

  @Override
  public Object getValue() {
    return value;
  }

  @Override
  public SpecificErrorBuilderBase<T> setValue(Object value) {
    this.value = value;
    hasValue = true;
    return this;
  }

  @Override
  public boolean hasValue() {
    return hasValue;
  }

  @Override
  public SpecificErrorBuilderBase<T> clearValue() {
    value = null;
    hasValue = false;
    return this;
  }

  @Override
  public Throwable getCause() {
    return cause;
  }

  @Override
  public SpecificErrorBuilderBase<T> setCause(Throwable cause) {
    this.cause = cause;
    hasCause = true;
    return this;
  }

  @Override
  public boolean hasCause() {
    return hasCause;
  }

  @Override
  public SpecificErrorBuilderBase<T> clearCause() {
    cause = null;
    hasCause = false;
    return this;
  }
}
