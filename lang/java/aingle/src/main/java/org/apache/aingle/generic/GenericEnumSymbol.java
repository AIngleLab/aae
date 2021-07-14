/*

 */
package org.apache.aingle.generic;

/** An enum symbol. */
public interface GenericEnumSymbol<E extends GenericEnumSymbol<E>> extends GenericContainer, Comparable<E> {
  /** Return the symbol. */
  @Override
  String toString();
}
