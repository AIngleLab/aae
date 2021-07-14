/*

 */

package org.apache.aingle.codegentest;

import org.apache.aingle.LogicalTypes;
import org.apache.aingle.codegentest.testdata.LogicalTypesWithCustomConversion;
import org.junit.BeforeClass;
import org.junit.Test;

import java.math.BigInteger;

public class TestCustomConversion extends AbstractSpecificRecordTest {

  @BeforeClass
  public static void init() {
    LogicalTypes.register(FixedSizeStringFactory.NAME, new FixedSizeStringFactory());
  }

  @Test
  public void testNullValues() {
    LogicalTypesWithCustomConversion instanceOfGeneratedClass = LogicalTypesWithCustomConversion.newBuilder()
        .setNonNullCustomField(new CustomDecimal(BigInteger.valueOf(100), 2))
        .setNonNullFixedSizeString(new FixedSizeString("test")).build();
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

  @Test
  public void testNonNullValues() {
    LogicalTypesWithCustomConversion instanceOfGeneratedClass = LogicalTypesWithCustomConversion.newBuilder()
        .setNonNullCustomField(new CustomDecimal(BigInteger.valueOf(100), 2))
        .setNullableCustomField(new CustomDecimal(BigInteger.valueOf(3000), 2))
        .setNonNullFixedSizeString(new FixedSizeString("test")).setNullableFixedSizeString(new FixedSizeString("test2"))
        .build();
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

  @Test(expected = IllegalArgumentException.class)
  public void testStringViolatesLimit() {
    LogicalTypesWithCustomConversion instanceOfGeneratedClass = LogicalTypesWithCustomConversion.newBuilder()
        .setNonNullCustomField(new CustomDecimal(BigInteger.valueOf(100), 2))
        .setNonNullFixedSizeString(new FixedSizeString("")).build();

    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }
}
