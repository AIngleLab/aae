/*

 */

package org.apache.aingle.codegentest;

import org.apache.aingle.codegentest.testdata.StringLogicalType;
import org.apache.aingle.generic.GenericData;
import org.junit.Test;

import java.util.UUID;

import static org.hamcrest.CoreMatchers.equalTo;
import static org.hamcrest.CoreMatchers.instanceOf;
import static org.hamcrest.MatcherAssert.assertThat;

public class TestLogicalTypeForStringType {

  /**
   * See AINGLE-2548: StringType of "String" causes logicalType converters to be
   * ignored for field
   */
  @Test
  public void shouldUseUUIDAsType() {
    StringLogicalType stringLogicalType = new StringLogicalType();
    stringLogicalType.setSomeIdentifier(UUID.randomUUID());
    assertThat(stringLogicalType.getSomeIdentifier(), instanceOf(UUID.class));
    assertThat(StringLogicalType.getClassSchema().getField("someJavaString").schema().getProp(GenericData.STRING_PROP),
        equalTo("String"));
  }

}
