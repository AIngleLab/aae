/*

 */

package org.apache.aingle.data;

import java.time.Instant;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import org.apache.aingle.Conversion;
import org.apache.aingle.LogicalTypes;
import org.apache.aingle.Schema;
import org.apache.aingle.data.TimeConversions.DateConversion;
import org.apache.aingle.data.TimeConversions.TimeMicrosConversion;
import org.apache.aingle.data.TimeConversions.TimeMillisConversion;
import org.apache.aingle.data.TimeConversions.TimestampMicrosConversion;
import org.apache.aingle.data.TimeConversions.TimestampMillisConversion;
import org.apache.aingle.reflect.ReflectData;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

public class TestTimeConversions {

  public static Schema DATE_SCHEMA;
  public static Schema TIME_MILLIS_SCHEMA;
  public static Schema TIME_MICROS_SCHEMA;
  public static Schema TIMESTAMP_MILLIS_SCHEMA;
  public static Schema TIMESTAMP_MICROS_SCHEMA;

  @BeforeClass
  public static void createSchemas() {
    TestTimeConversions.DATE_SCHEMA = LogicalTypes.date().addToSchema(Schema.create(Schema.Type.INT));
    TestTimeConversions.TIME_MILLIS_SCHEMA = LogicalTypes.timeMillis().addToSchema(Schema.create(Schema.Type.INT));
    TestTimeConversions.TIME_MICROS_SCHEMA = LogicalTypes.timeMicros().addToSchema(Schema.create(Schema.Type.LONG));
    TestTimeConversions.TIMESTAMP_MILLIS_SCHEMA = LogicalTypes.timestampMillis()
        .addToSchema(Schema.create(Schema.Type.LONG));
    TestTimeConversions.TIMESTAMP_MICROS_SCHEMA = LogicalTypes.timestampMicros()
        .addToSchema(Schema.create(Schema.Type.LONG));
  }

  @Test
  public void testDateConversion() throws Exception {
    DateConversion conversion = new DateConversion();
    LocalDate Jan_6_1970 = LocalDate.of(1970, 1, 6); // 5
    LocalDate Jan_1_1970 = LocalDate.of(1970, 1, 1); // 0
    LocalDate Dec_27_1969 = LocalDate.of(1969, 12, 27); // -5

    Assert.assertEquals("6 Jan 1970 should be 5", 5,
        (int) conversion.toInt(Jan_6_1970, DATE_SCHEMA, LogicalTypes.date()));
    Assert.assertEquals("1 Jan 1970 should be 0", 0,
        (int) conversion.toInt(Jan_1_1970, DATE_SCHEMA, LogicalTypes.date()));
    Assert.assertEquals("27 Dec 1969 should be -5", -5,
        (int) conversion.toInt(Dec_27_1969, DATE_SCHEMA, LogicalTypes.date()));

    Assert.assertEquals("6 Jan 1970 should be 5", conversion.fromInt(5, DATE_SCHEMA, LogicalTypes.date()), Jan_6_1970);
    Assert.assertEquals("1 Jan 1970 should be 0", conversion.fromInt(0, DATE_SCHEMA, LogicalTypes.date()), Jan_1_1970);
    Assert.assertEquals("27 Dec 1969 should be -5", conversion.fromInt(-5, DATE_SCHEMA, LogicalTypes.date()),
        Dec_27_1969);
  }

  @Test
  public void testTimeMillisConversion() {
    TimeMillisConversion conversion = new TimeMillisConversion();
    LocalTime oneAM = LocalTime.of(1, 0);
    LocalTime afternoon = LocalTime.of(15, 14, 15, 926_000_000);
    int afternoonMillis = ((15 * 60 + 14) * 60 + 15) * 1000 + 926;

    Assert.assertEquals("Midnight should be 0", 0,
        (int) conversion.toInt(LocalTime.MIDNIGHT, TIME_MILLIS_SCHEMA, LogicalTypes.timeMillis()));
    Assert.assertEquals("01:00 should be 3,600,000", 3_600_000,
        (int) conversion.toInt(oneAM, TIME_MILLIS_SCHEMA, LogicalTypes.timeMillis()));
    Assert.assertEquals("15:14:15.926 should be " + afternoonMillis, afternoonMillis,
        (int) conversion.toInt(afternoon, TIME_MILLIS_SCHEMA, LogicalTypes.timeMillis()));

    Assert.assertEquals("Midnight should be 0", LocalTime.MIDNIGHT,
        conversion.fromInt(0, TIME_MILLIS_SCHEMA, LogicalTypes.timeMillis()));
    Assert.assertEquals("01:00 should be 3,600,000", oneAM,
        conversion.fromInt(3600000, TIME_MILLIS_SCHEMA, LogicalTypes.timeMillis()));
    Assert.assertEquals("15:14:15.926 should be " + afternoonMillis, afternoon,
        conversion.fromInt(afternoonMillis, TIME_MILLIS_SCHEMA, LogicalTypes.timeMillis()));
  }

  @Test
  public void testTimeMicrosConversion() throws Exception {
    TimeMicrosConversion conversion = new TimeMicrosConversion();
    LocalTime oneAM = LocalTime.of(1, 0);
    LocalTime afternoon = LocalTime.of(15, 14, 15, 926_551_000);
    long afternoonMicros = ((long) (15 * 60 + 14) * 60 + 15) * 1_000_000 + 926_551;

    Assert.assertEquals("Midnight should be 0", LocalTime.MIDNIGHT,
        conversion.fromLong(0L, TIME_MICROS_SCHEMA, LogicalTypes.timeMicros()));
    Assert.assertEquals("01:00 should be 3,600,000,000", oneAM,
        conversion.fromLong(3_600_000_000L, TIME_MICROS_SCHEMA, LogicalTypes.timeMicros()));
    Assert.assertEquals("15:14:15.926551 should be " + afternoonMicros, afternoon,
        conversion.fromLong(afternoonMicros, TIME_MICROS_SCHEMA, LogicalTypes.timeMicros()));

    Assert.assertEquals("Midnight should be 0", 0,
        (long) conversion.toLong(LocalTime.MIDNIGHT, TIME_MICROS_SCHEMA, LogicalTypes.timeMicros()));
    Assert.assertEquals("01:00 should be 3,600,000,000", 3_600_000_000L,
        (long) conversion.toLong(oneAM, TIME_MICROS_SCHEMA, LogicalTypes.timeMicros()));
    Assert.assertEquals("15:14:15.926551 should be " + afternoonMicros, afternoonMicros,
        (long) conversion.toLong(afternoon, TIME_MICROS_SCHEMA, LogicalTypes.timeMicros()));
  }

  @Test
  public void testTimestampMillisConversion() throws Exception {
    TimestampMillisConversion conversion = new TimestampMillisConversion();
    long nowInstant = Instant.now().toEpochMilli(); // ms precision

    // round trip
    Instant now = conversion.fromLong(nowInstant, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis());
    long roundTrip = conversion.toLong(now, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis());
    Assert.assertEquals("Round-trip conversion should work", nowInstant, roundTrip);

    long May_28_2015_21_46_53_221_instant = 1432849613221L;
    Instant May_28_2015_21_46_53_221 = ZonedDateTime.of(2015, 5, 28, 21, 46, 53, 221_000_000, ZoneOffset.UTC)
        .toInstant();

    // known dates from https://www.epochconverter.com/
    // > Epoch
    Assert.assertEquals("Known date should be correct", May_28_2015_21_46_53_221,
        conversion.fromLong(May_28_2015_21_46_53_221_instant, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));
    Assert.assertEquals("Known date should be correct", May_28_2015_21_46_53_221_instant,
        (long) conversion.toLong(May_28_2015_21_46_53_221, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));

    // Epoch
    Assert.assertEquals("1970-01-01 should be 0", Instant.EPOCH,
        conversion.fromLong(0L, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));
    Assert.assertEquals("1970-01-01 should be 0", 0L,
        (long) conversion.toLong(ZonedDateTime.ofInstant(Instant.EPOCH, ZoneOffset.UTC).toInstant(),
            TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));

    // < Epoch
    long Jul_01_1969_12_00_00_123_instant = -15854400000L + 123;
    Instant Jul_01_1969_12_00_00_123 = ZonedDateTime.of(1969, 7, 1, 12, 0, 0, 123_000_000, ZoneOffset.UTC).toInstant();

    Assert.assertEquals("Pre 1970 date should be correct", Jul_01_1969_12_00_00_123,
        conversion.fromLong(Jul_01_1969_12_00_00_123_instant, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));
    Assert.assertEquals("Pre 1970 date should be correct", Jul_01_1969_12_00_00_123_instant,
        (long) conversion.toLong(Jul_01_1969_12_00_00_123, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));
  }

  @Test
  public void testTimestampMicrosConversion() throws Exception {
    TimestampMicrosConversion conversion = new TimestampMicrosConversion();

    // known dates from https://www.epochconverter.com/
    // > Epoch
    long May_28_2015_21_46_53_221_843_instant = 1432849613221L * 1000 + 843;
    Instant May_28_2015_21_46_53_221_843 = ZonedDateTime.of(2015, 5, 28, 21, 46, 53, 221_843_000, ZoneOffset.UTC)
        .toInstant();

    Assert.assertEquals("Known date should be correct", May_28_2015_21_46_53_221_843, conversion
        .fromLong(May_28_2015_21_46_53_221_843_instant, TIMESTAMP_MICROS_SCHEMA, LogicalTypes.timestampMicros()));

    Assert.assertEquals("Known date should be correct", May_28_2015_21_46_53_221_843_instant, (long) conversion
        .toLong(May_28_2015_21_46_53_221_843, TIMESTAMP_MICROS_SCHEMA, LogicalTypes.timestampMillis()));

    // Epoch
    Assert.assertEquals("1970-01-01 should be 0", Instant.EPOCH,
        conversion.fromLong(0L, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));
    Assert.assertEquals("1970-01-01 should be 0", 0L,
        (long) conversion.toLong(ZonedDateTime.ofInstant(Instant.EPOCH, ZoneOffset.UTC).toInstant(),
            TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));

    // < Epoch
    long Jul_01_1969_12_00_00_000_123_instant = -15854400000L * 1000 + 123;
    Instant Jul_01_1969_12_00_00_000_123 = ZonedDateTime.of(1969, 7, 1, 12, 0, 0, 123_000, ZoneOffset.UTC).toInstant();

    Assert.assertEquals("Pre 1970 date should be correct", Jul_01_1969_12_00_00_000_123, conversion
        .fromLong(Jul_01_1969_12_00_00_000_123_instant, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));
    Assert.assertEquals("Pre 1970 date should be correct", Jul_01_1969_12_00_00_000_123_instant, (long) conversion
        .toLong(Jul_01_1969_12_00_00_000_123, TIMESTAMP_MILLIS_SCHEMA, LogicalTypes.timestampMillis()));
  }

  @Test
  public void testDynamicSchemaWithDateConversion() throws ClassNotFoundException {
    Schema schema = getReflectedSchemaByName("java.time.LocalDate", new TimeConversions.DateConversion());
    Assert.assertEquals("Reflected schema should be logicalType date", DATE_SCHEMA, schema);
  }

  @Test
  public void testDynamicSchemaWithTimeConversion() throws ClassNotFoundException {
    Schema schema = getReflectedSchemaByName("java.time.LocalTime", new TimeConversions.TimeMillisConversion());
    Assert.assertEquals("Reflected schema should be logicalType timeMillis", TIME_MILLIS_SCHEMA, schema);
  }

  @Test
  public void testDynamicSchemaWithTimeMicrosConversion() throws ClassNotFoundException {
    Schema schema = getReflectedSchemaByName("java.time.LocalTime", new TimeConversions.TimeMicrosConversion());
    Assert.assertEquals("Reflected schema should be logicalType timeMicros", TIME_MICROS_SCHEMA, schema);
  }

  @Test
  public void testDynamicSchemaWithDateTimeConversion() throws ClassNotFoundException {
    Schema schema = getReflectedSchemaByName("java.time.Instant", new TimeConversions.TimestampMillisConversion());
    Assert.assertEquals("Reflected schema should be logicalType timestampMillis", TIMESTAMP_MILLIS_SCHEMA, schema);
  }

  @Test
  public void testDynamicSchemaWithDateTimeMicrosConversion() throws ClassNotFoundException {
    Schema schema = getReflectedSchemaByName("java.time.Instant", new TimeConversions.TimestampMicrosConversion());
    Assert.assertEquals("Reflected schema should be logicalType timestampMicros", TIMESTAMP_MICROS_SCHEMA, schema);
  }

  private Schema getReflectedSchemaByName(String className, Conversion<?> conversion) throws ClassNotFoundException {
    // one argument: a fully qualified class name
    Class<?> cls = Class.forName(className);

    // get the reflected schema for the given class
    ReflectData model = new ReflectData();
    model.addLogicalTypeConversion(conversion);
    return model.getSchema(cls);
  }
}
