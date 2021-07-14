/*

 */

package org.apache.aingle.mapred;

/** The wrapper of values for jobs configured with {@link AIngleJob} . */
public class AIngleValue<T> extends AIngleWrapper<T> {
  /** Wrap null. Construct {@link AIngleValue} wrapping no value. */
  public AIngleValue() {
    this(null);
  }

  /** Wrap a value. */
  public AIngleValue(T datum) {
    super(datum);
  }
}
