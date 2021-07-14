/*

 */
package org.apache.aingle.compiler.schema;

import org.apache.aingle.Schema;

public interface SchemaVisitor<T> {

  /**
   * Invoked for schemas that do not have "child" schemas (like string, int ...)
   * or for a previously encountered schema with children, which will be treated
   * as a terminal. (to avoid circular recursion)
   *
   * @param terminal
   */
  SchemaVisitorAction visitTerminal(Schema terminal);

  /**
   * Invoked for schema with children before proceeding to visit the children.
   *
   * @param nonTerminal
   */
  SchemaVisitorAction visitNonTerminal(Schema nonTerminal);

  /**
   * Invoked for schemas with children after its children have been visited.
   *
   * @param nonTerminal
   */
  SchemaVisitorAction afterVisitNonTerminal(Schema nonTerminal);

  /**
   * Invoked when visiting is complete.
   *
   * @return a value which will be returned by the visit method.
   */
  T get();

}
