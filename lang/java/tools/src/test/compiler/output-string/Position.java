/**
 * Autogenerated by AIngle
 *
 * DO NOT EDIT DIRECTLY
 */
package aingle.examples.baseball;
@org.apache.aingle.specific.AIngleGenerated
public enum Position implements org.apache.aingle.generic.GenericEnumSymbol<Position> {
  P, C, B1, B2, B3, SS, LF, CF, RF, DH  ;
  public static final org.apache.aingle.Schema SCHEMA$ = new org.apache.aingle.Schema.Parser().parse("{\"type\":\"enum\",\"name\":\"Position\",\"namespace\":\"aingle.examples.baseball\",\"symbols\":[\"P\",\"C\",\"B1\",\"B2\",\"B3\",\"SS\",\"LF\",\"CF\",\"RF\",\"DH\"]}");
  public static org.apache.aingle.Schema getClassSchema() { return SCHEMA$; }
  public org.apache.aingle.Schema getSchema() { return SCHEMA$; }
}
