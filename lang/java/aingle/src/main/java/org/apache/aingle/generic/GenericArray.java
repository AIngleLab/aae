/*

 */

package org.apache.aingle.generic;

import java.util.List;

/** Array that permits reuse of contained elements. */
public interface GenericArray<T> extends List<T>, GenericContainer {
  /**
   * The current content of the location where {@link #add(Object)} would next
   * store an element, if any. This permits reuse of arrays and their elements
   * without allocating new objects.
   */
  T peek();

  /** reset size counter of array to zero */
  default void reset() {
    clear();
  }

  /** clean up reusable objects from array (if reset didn't already) */
  default void prune() {
  }

  /** Reverses the order of the elements in this array. */
  void reverse();
}
