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
public class TestSchemaCompatibilityFixedSizeMismatch {

  @Parameters(name = "r: {0} | w: {1}")
  public static Iterable<Object[]> data() {
    Object[][] fields = { //
        { FIXED_4_BYTES, FIXED_8_BYTES, "expected: 8, found: 4", "/size" },
        { FIXED_8_BYTES, FIXED_4_BYTES, "expected: 4, found: 8", "/size" },
        { A_DINT_B_DFIXED_8_BYTES_RECORD1, A_DINT_B_DFIXED_4_BYTES_RECORD1, "expected: 4, found: 8",
            "/fields/1/type/size" },
        { A_DINT_B_DFIXED_4_BYTES_RECORD1, A_DINT_B_DFIXED_8_BYTES_RECORD1, "expected: 8, found: 4",
            "/fields/1/type/size" }, };
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
  public void testFixedSizeMismatchSchemas() throws Exception {
    validateIncompatibleSchemas(reader, writer, SchemaIncompatibilityType.FIXED_SIZE_MISMATCH, details, location);
  }
}
