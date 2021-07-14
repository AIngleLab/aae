/*

 */
package org.apache.aingle.codegentest;

import org.apache.aingle.codegentest.testdata.NestedLogicalTypesArray;
import org.apache.aingle.codegentest.testdata.NestedLogicalTypesMap;
import org.apache.aingle.codegentest.testdata.NestedLogicalTypesRecord;
import org.apache.aingle.codegentest.testdata.NestedLogicalTypesUnion;
import org.apache.aingle.codegentest.testdata.NestedLogicalTypesUnionFixedDecimal;
import org.apache.aingle.codegentest.testdata.NestedRecord;
import org.apache.aingle.codegentest.testdata.NullableLogicalTypesArray;
import org.apache.aingle.codegentest.testdata.RecordInArray;
import org.apache.aingle.codegentest.testdata.RecordInMap;
import org.apache.aingle.codegentest.testdata.RecordInUnion;
import org.junit.Test;

import java.math.BigInteger;
import java.time.LocalDate;
import java.util.Collections;

public class TestNestedLogicalTypes extends AbstractSpecificRecordTest {

  @Test
  public void testNullableLogicalTypeInNestedRecord() {
    final NestedLogicalTypesRecord nestedLogicalTypesRecord = NestedLogicalTypesRecord.newBuilder()
        .setNestedRecord(NestedRecord.newBuilder().setNullableDateField(LocalDate.now()).build()).build();
    verifySerDeAndStandardMethods(nestedLogicalTypesRecord);
  }

  @Test
  public void testNullableLogicalTypeInArray() {
    final NullableLogicalTypesArray logicalTypesArray = NullableLogicalTypesArray.newBuilder()
        .setArrayOfLogicalType(Collections.singletonList(LocalDate.now())).build();
    verifySerDeAndStandardMethods(logicalTypesArray);
  }

  @Test
  public void testNullableLogicalTypeInRecordInArray() {
    final NestedLogicalTypesArray nestedLogicalTypesArray = NestedLogicalTypesArray.newBuilder()
        .setArrayOfRecords(
            Collections.singletonList(RecordInArray.newBuilder().setNullableDateField(LocalDate.now()).build()))
        .build();
    verifySerDeAndStandardMethods(nestedLogicalTypesArray);
  }

  @Test
  public void testNullableLogicalTypeInRecordInUnion() {
    final NestedLogicalTypesUnion nestedLogicalTypesUnion = NestedLogicalTypesUnion.newBuilder()
        .setUnionOfRecords(RecordInUnion.newBuilder().setNullableDateField(LocalDate.now()).build()).build();
    verifySerDeAndStandardMethods(nestedLogicalTypesUnion);
  }

  @Test
  public void testNullableLogicalTypeInRecordInMap() {
    final NestedLogicalTypesMap nestedLogicalTypesMap = NestedLogicalTypesMap.newBuilder()
        .setMapOfRecords(
            Collections.singletonMap("key", RecordInMap.newBuilder().setNullableDateField(LocalDate.now()).build()))
        .build();
    verifySerDeAndStandardMethods(nestedLogicalTypesMap);
  }

  @Test
  public void testNullableLogicalTypeInRecordInFixedDecimal() {
    final NestedLogicalTypesUnionFixedDecimal nestedLogicalTypesUnionFixedDecimal = NestedLogicalTypesUnionFixedDecimal
        .newBuilder().setUnionOfFixedDecimal(new CustomDecimal(BigInteger.TEN, 15)).build();
    verifySerDeAndStandardMethods(nestedLogicalTypesUnionFixedDecimal);
  }

}
