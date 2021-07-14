/*

 */

package org.apache.aingle;

/** Thrown when the expected contents of a union cannot be resolved. */
public class UnresolvedUnionException extends AIngleRuntimeException {
  private Object unresolvedDatum;
  private Schema unionSchema;

  public UnresolvedUnionException(Schema unionSchema, Object unresolvedDatum) {
    super("Not in union " + unionSchema + ": " + unresolvedDatum);
    this.unionSchema = unionSchema;
    this.unresolvedDatum = unresolvedDatum;
  }

  public UnresolvedUnionException(Schema unionSchema, Schema.Field field, Object unresolvedDatum) {
    super("Not in union " + unionSchema + ": " + unresolvedDatum + " (field=" + field.name() + ")");
    this.unionSchema = unionSchema;
    this.unresolvedDatum = unresolvedDatum;
  }

  public Object getUnresolvedDatum() {
    return unresolvedDatum;
  }

  public Schema getUnionSchema() {
    return unionSchema;
  }
}
