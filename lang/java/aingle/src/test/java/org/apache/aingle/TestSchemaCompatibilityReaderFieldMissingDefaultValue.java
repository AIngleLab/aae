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
public class TestSchemaCompatibilityReaderFieldMissingDefaultValue {
  @Parameters(name = "r: {0} | w: {1}")
  public static Iterable<Object[]> data() {
    Object[][] fields = { //
        { A_INT_RECORD1, EMPTY_RECORD1, "a", "/fields/0" }, { A_INT_B_DINT_RECORD1, EMPTY_RECORD1, "a", "/fields/0" } };
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
  public void testReaderFieldMissingDefaultValueSchemas() throws Exception {
    validateIncompatibleSchemas(reader, writer, SchemaIncompatibilityType.READER_FIELD_MISSING_DEFAULT_VALUE, details,
        location);
  }
}
