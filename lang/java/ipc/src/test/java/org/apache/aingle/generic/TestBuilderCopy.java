/*

 */

package org.apache.aingle.generic;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.HashMap;

import org.junit.Test;

import test.StringablesRecord;

import static org.junit.Assert.assertEquals;

/** Unit test for performing a builder copy of an object with a schema */
public class TestBuilderCopy {
  @Test
  public void testBuilderCopy() {
    StringablesRecord.Builder builder = StringablesRecord.newBuilder();
    builder.setValue(new BigDecimal("1314.11"));

    HashMap<String, BigDecimal> mapWithBigDecimalElements = new HashMap<>();
    mapWithBigDecimalElements.put("testElement", new BigDecimal("220.11"));
    builder.setMapWithBigDecimalElements(mapWithBigDecimalElements);

    HashMap<BigInteger, String> mapWithBigIntKeys = new HashMap<>();
    mapWithBigIntKeys.put(BigInteger.ONE, "testKey");
    builder.setMapWithBigIntKeys(mapWithBigIntKeys);

    StringablesRecord original = builder.build();

    StringablesRecord duplicate = StringablesRecord.newBuilder(original).build();

    assertEquals(duplicate, original);
  }
}
