/*

 */

package org.apache.aingle.codegentest;

import java.time.LocalDate;

import org.apache.aingle.codegentest.testdata.LogicalTypesWithDefaults;
import org.junit.Assert;
import org.junit.Test;

import java.io.IOException;

public class TestLogicalTypesWithDefaults extends AbstractSpecificRecordTest {

  private static final LocalDate DEFAULT_VALUE = LocalDate.parse("1973-05-19");

  @Test
  public void testDefaultValueOfNullableField() throws IOException {
    LogicalTypesWithDefaults instanceOfGeneratedClass = LogicalTypesWithDefaults.newBuilder()
        .setNonNullDate(LocalDate.now()).build();
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

  @Test
  public void testDefaultValueOfNonNullField() throws IOException {
    LogicalTypesWithDefaults instanceOfGeneratedClass = LogicalTypesWithDefaults.newBuilder()
        .setNullableDate(LocalDate.now()).build();
    Assert.assertEquals(DEFAULT_VALUE, instanceOfGeneratedClass.getNonNullDate());
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

  @Test
  public void testWithValues() throws IOException {
    LogicalTypesWithDefaults instanceOfGeneratedClass = LogicalTypesWithDefaults.newBuilder()
        .setNullableDate(LocalDate.now()).setNonNullDate(LocalDate.now()).build();
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

}
