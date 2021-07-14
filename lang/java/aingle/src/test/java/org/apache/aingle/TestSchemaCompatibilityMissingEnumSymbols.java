/*

 */
package org.apache.aingle;

import static org.apache.aingle.TestSchemaCompatibility.validateIncompatibleSchemas;
import static org.apache.aingle.TestSchemas.*;

import java.util.Arrays;

import org.apache.aingle.SchemaCompatibility.SchemaIncompatibilityType;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameter;
import org.junit.runners.Parameterized.Parameters;

@RunWith(Parameterized.class)
public class TestSchemaCompatibilityMissingEnumSymbols {

  private static final Schema RECORD1_WITH_ENUM_AB = SchemaBuilder.record("Record1").fields() //
      .name("field1").type(ENUM1_AB_SCHEMA).noDefault() //
      .endRecord();
  private static final Schema RECORD1_WITH_ENUM_ABC = SchemaBuilder.record("Record1").fields() //
      .name("field1").type(ENUM1_ABC_SCHEMA).noDefault() //
      .endRecord();

  @Parameters(name = "r: {0} | w: {1}")
  public static Iterable<Object[]> data() {
    Object[][] fields = { //
        { ENUM1_AB_SCHEMA, ENUM1_ABC_SCHEMA, "[C]", "/symbols" },
        { ENUM1_BC_SCHEMA, ENUM1_ABC_SCHEMA, "[A]", "/symbols" },
        { RECORD1_WITH_ENUM_AB, RECORD1_WITH_ENUM_ABC, "[C]", "/fields/0/type/symbols" } };
    return Arrays.asList(fields);
  }

  @Parameter(0)
  public Schema reader;
  @Parameter(1)
  public Schema writer;
  @Parameter(2)
  public String details;
  @Parameter(3)
  public String location;

  @Test
  public void testTypeMismatchSchemas() throws Exception {
    validateIncompatibleSchemas(reader, writer, SchemaIncompatibilityType.MISSING_ENUM_SYMBOLS, details, location);
  }
}
