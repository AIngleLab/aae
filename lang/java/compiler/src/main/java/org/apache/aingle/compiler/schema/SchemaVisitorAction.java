/*

 */

package org.apache.aingle.compiler.schema;

public enum SchemaVisitorAction {

  /**
   * continue visit.
   */
  CONTINUE,
  /**
   * terminate visit.
   */
  TERMINATE,
  /**
   * when returned from pre non terminal visit method the children of the non
   * terminal are skipped. afterVisitNonTerminal for the current schema will not
   * be invoked.
   */
  SKIP_SUBTREE,
  /**
   * Skip visiting the siblings of this schema.
   */
  SKIP_SIBLINGS;

}
