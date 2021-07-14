/*

 */

package org.apache.aingle.codegentest;

import java.time.LocalDate;
import org.apache.aingle.codegentest.testdata.NullableLogicalTypes;
import org.junit.Test;

import java.io.IOException;

public class TestNullableLogicalTypes extends AbstractSpecificRecordTest {

  @Test
  public void testWithNullValues() throws IOException {
    NullableLogicalTypes instanceOfGeneratedClass = NullableLogicalTypes.newBuilder().setNullableDate(null).build();
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

  @Test
  public void testDate() throws IOException {
    NullableLogicalTypes instanceOfGeneratedClass = NullableLogicalTypes.newBuilder().setNullableDate(LocalDate.now())
        .build();
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

}
