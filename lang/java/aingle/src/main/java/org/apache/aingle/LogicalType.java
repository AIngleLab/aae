/*

 */

package org.apache.aingle;

import org.apache.aingle.generic.GenericData;
import org.apache.aingle.specific.SpecificData;

/**
 * Logical types provides an opt-in way to extend AIngle's types. Logical types
 * specify a way of representing a high-level type as a base AIngle type. For
 * example, a date is specified as the number of days after the unix epoch (or
 * before using a negative value). This enables extensions to AIngle's type system
 * without breaking binary compatibility. Older versions see the base type and
 * ignore the logical type.
 */
public class LogicalType {

  public static final String LOGICAL_TYPE_PROP = "logicalType";

  private static final String[] INCOMPATIBLE_PROPS = new String[] { GenericData.STRING_PROP, SpecificData.CLASS_PROP,
      SpecificData.KEY_CLASS_PROP, SpecificData.ELEMENT_PROP };

  private final String name;

  public LogicalType(String logicalTypeName) {
    this.name = logicalTypeName.intern();
  }

  /**
   * Get the name of this logical type.
   * <p>
   * This name is set as the Schema property "logicalType".
   *
   * @return the String name of the logical type
   */
  public String getName() {
    return name;
  }

  /**
   * Add this logical type to the given Schema.
   * <p>
   * The "logicalType" property will be set to this type's name, and other
   * type-specific properties may be added. The Schema is first validated to
   * ensure it is compatible.
   *
   * @param schema a Schema
   * @return the modified Schema
   * @throws IllegalArgumentException if the type and schema are incompatible
   */
  public Schema addToSchema(Schema schema) {
    validate(schema);
    schema.addProp(LOGICAL_TYPE_PROP, name);
    schema.setLogicalType(this);
    return schema;
  }

  /**
   * Validate this logical type for the given Schema.
   * <p>
   * This will throw an exception if the Schema is incompatible with this type.
   * For example, a date is stored as an int and is incompatible with a fixed
   * Schema.
   *
   * @param schema a Schema
   * @throws IllegalArgumentException if the type and schema are incompatible
   */
  public void validate(Schema schema) {
    for (String incompatible : INCOMPATIBLE_PROPS) {
      if (schema.getProp(incompatible) != null) {
        throw new IllegalArgumentException(LOGICAL_TYPE_PROP + " cannot be used with " + incompatible);
      }
    }
  }

}
