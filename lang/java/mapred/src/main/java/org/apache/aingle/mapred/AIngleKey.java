/*

 */

package org.apache.aingle.mapred;

/** The wrapper of keys for jobs configured with {@link AIngleJob} . */
public class AIngleKey<T> extends AIngleWrapper<T> {
  /** Wrap null. Construct {@link AIngleKey} wrapping no key. */
  public AIngleKey() {
    this(null);
  }

  /** Wrap a key. */
  public AIngleKey(T datum) {
    super(datum);
  }
}
