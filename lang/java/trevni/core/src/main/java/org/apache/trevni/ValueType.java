/*

 */
package org.apache.trevni;

/** The datatypes that may be stored in a column. */
public enum ValueType {
  NULL, BOOLEAN, INT, LONG, FIXED32, FIXED64, FLOAT, DOUBLE, STRING, BYTES;

  private final String name;

  private ValueType() {
    this.name = this.name().toLowerCase();
  }

  /** Return the name of this type. */
  public String getName() {
    return name;
  }

  /** Return a type given its name. */
  public static ValueType forName(String name) {
    return valueOf(name.toUpperCase());
  }

}
