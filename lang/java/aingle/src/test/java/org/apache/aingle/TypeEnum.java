/**
 * Autogenerated by AIngle
 *
 * DO NOT EDIT DIRECTLY
 *

 */
package org.apache.aingle;

@SuppressWarnings("all")
@org.apache.aingle.specific.AIngleGenerated
public enum TypeEnum implements org.apache.aingle.generic.GenericEnumSymbol<TypeEnum> {
  a, b, c;

  public static final org.apache.aingle.Schema SCHEMA$ = new org.apache.aingle.Schema.Parser().parse(
      "{\"type\":\"enum\",\"name\":\"TypeEnum\",\"namespace\":\"org.apache.aingle\",\"symbols\":[\"a\",\"b\",\"c\"]}");

  public static org.apache.aingle.Schema getClassSchema() {
    return SCHEMA$;
  }

  public org.apache.aingle.Schema getSchema() {
    return SCHEMA$;
  }
}
