/*

 */
package org.apache.aingle;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;

import static org.apache.aingle.LogicalType.LOGICAL_TYPE_PROP;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.nullValue;

public class TestSchemaWarnings {

  private final static PrintStream originalErr = System.err;

  /**
   * The capturable replacement for the system err stream.
   */
  private final ByteArrayOutputStream capturedErr = new ByteArrayOutputStream();

  @Before
  public void setupStdErr() {
    capturedErr.reset();
    System.setErr(new PrintStream(capturedErr));
  }

  @AfterClass
  public static void restoreStdErr() {
    System.setErr(originalErr);
  }

  public String getCapturedStdErr() {
    System.out.flush();
    String stderr = new String(capturedErr.toByteArray(), StandardCharsets.UTF_8);
    capturedErr.reset();
    return stderr;
  }

  @Test
  public void testWarnWhenTheLogicalTypeIsOnTheField() {
    // A record with a single int field.
    Schema s = SchemaBuilder.record("A").fields().requiredInt("a1").endRecord();

    // Force reparsing the schema, and no warning should be logged.
    s = new Schema.Parser().parse(s.toString());
    assertThat(s.getField("a1").schema().getLogicalType(), nullValue());
    assertThat(getCapturedStdErr(), is(""));

    // Add the logical type annotation to the field (as opposed to the field schema)
    // and parse it again. This is a common error, see AINGLE-3014, AINGLE-2015.
    s.getField("a1").addProp(LOGICAL_TYPE_PROP, LogicalTypes.date().getName());
    assertThat(s.getField("a1").schema().getLogicalType(), nullValue());

    // Force reparsing the schema, and a warning should be logged.
    s = new Schema.Parser().parse(s.toString());
    assertThat(getCapturedStdErr(), containsString("Ignored the A.a1.logicalType property (\"date\"). It should"
        + " probably be nested inside the \"type\" for the field."));
    assertThat(s.getField("a1").schema().getLogicalType(), nullValue());

    // Add the logical type annotation to the field schema. This doesn't change the
    // logical type of an already parsed schema.
    s.getField("a1").schema().addProp(LOGICAL_TYPE_PROP, LogicalTypes.date().getName());
    assertThat(s.getField("a1").schema().getLogicalType(), nullValue());

    // Force reparsing the schema. No warning should be logged, and the logical type
    // should be applied.
    s = new Schema.Parser().parse(s.toString());
    assertThat(getCapturedStdErr(), is(""));
    assertThat(s.getField("a1").schema().getLogicalType(), is(LogicalTypes.date()));

  }

  @Test
  public void testWarnWhenTheLogicalTypeIsIgnored() {
    // A record with a single int field.
    Schema s = SchemaBuilder.record("A").fields().requiredLong("a1").endRecord();

    // Add the logical type annotation to the field (as opposed to the field schema)
    // and parse it again.
    s.getField("a1").schema().addProp(LOGICAL_TYPE_PROP, LogicalTypes.date().getName());
    // Force reparsing the schema. No warning should be logged, and the logical type
    // should be applied.
    s = new Schema.Parser().parse(s.toString());
    assertThat(s.getField("a1").schema().getLogicalType(), nullValue());
    assertThat(getCapturedStdErr(), containsString("Ignoring invalid logical type for name: date"));
  }
}
