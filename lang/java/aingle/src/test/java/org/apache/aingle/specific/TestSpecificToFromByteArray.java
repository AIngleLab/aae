/*

 */
package org.apache.aingle.specific;

import org.apache.aingle.Conversions;
import org.apache.aingle.LogicalTypes;
import org.apache.aingle.message.MissingSchemaException;
import org.junit.Test;

import java.io.IOException;
import java.math.BigDecimal;
import java.nio.ByteBuffer;
import java.time.Instant;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.temporal.ChronoUnit;

import static org.junit.Assert.assertEquals;

public class TestSpecificToFromByteArray {

  @Test
  public void testSpecificToFromByteBufferWithLogicalTypes() throws IOException {
    // Java 9+ comes with NANO precision and since we encode it using millis
    // precision
    // Therefore we won't want to have NANOs in the input
    LocalTime t = LocalTime.now().truncatedTo(ChronoUnit.MILLIS);
    Instant instant = Instant.now().truncatedTo(ChronoUnit.MILLIS);

    final TestRecordWithLogicalTypes record = new TestRecordWithLogicalTypes(true, 34, 35L, 3.14F, 3019.34, null,
        LocalDate.now(), t, instant, new BigDecimal("123.45"));

    final ByteBuffer b = record.toByteBuffer();
    final TestRecordWithLogicalTypes copy = TestRecordWithLogicalTypes.fromByteBuffer(b);

    assertEquals(record, copy);
  }

  @Test
  public void testSpecificToFromByteBufferWithoutLogicalTypes() throws IOException {
    final TestRecordWithoutLogicalTypes record = new TestRecordWithoutLogicalTypes(true, 34, 35L, 3.14F, 3019.34, null,
        (int) System.currentTimeMillis() / 1000, (int) System.currentTimeMillis() / 1000, System.currentTimeMillis(),
        new Conversions.DecimalConversion().toBytes(new BigDecimal("123.45"), null, LogicalTypes.decimal(9, 2)));

    final ByteBuffer b = record.toByteBuffer();
    final TestRecordWithoutLogicalTypes copy = TestRecordWithoutLogicalTypes.fromByteBuffer(b);

    assertEquals(record, copy);
  }

  @Test(expected = MissingSchemaException.class)
  public void testSpecificByteArrayIncompatibleWithLogicalTypes() throws IOException {
    final TestRecordWithoutLogicalTypes withoutLogicalTypes = new TestRecordWithoutLogicalTypes(true, 34, 35L, 3.14F,
        3019.34, null, (int) System.currentTimeMillis() / 1000, (int) System.currentTimeMillis() / 1000,
        System.currentTimeMillis(),
        new Conversions.DecimalConversion().toBytes(new BigDecimal("123.45"), null, LogicalTypes.decimal(9, 2)));

    final ByteBuffer b = withoutLogicalTypes.toByteBuffer();
    TestRecordWithLogicalTypes.fromByteBuffer(b);
  }

  @Test(expected = MissingSchemaException.class)
  public void testSpecificByteArrayIncompatibleWithoutLogicalTypes() throws IOException {
    final TestRecordWithLogicalTypes withLogicalTypes = new TestRecordWithLogicalTypes(true, 34, 35L, 3.14F, 3019.34,
        null, LocalDate.now(), LocalTime.now(), Instant.now(), new BigDecimal("123.45"));

    final ByteBuffer b = withLogicalTypes.toByteBuffer();
    TestRecordWithoutLogicalTypes.fromByteBuffer(b);
  }
}
