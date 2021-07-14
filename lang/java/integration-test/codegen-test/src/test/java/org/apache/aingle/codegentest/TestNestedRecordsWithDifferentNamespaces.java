/*

 */

package org.apache.aingle.codegentest;

import org.apache.aingle.codegentest.other.NestedOtherNamespaceRecord;
import org.apache.aingle.codegentest.some.NestedSomeNamespaceRecord;
import org.junit.Test;

public class TestNestedRecordsWithDifferentNamespaces extends AbstractSpecificRecordTest {

  @Test
  public void testNestedRecordsWithDifferentNamespaces() {
    NestedSomeNamespaceRecord instanceOfGeneratedClass = NestedSomeNamespaceRecord.newBuilder()
        .setNestedRecordBuilder(NestedOtherNamespaceRecord.newBuilder().setSomeField(1)).build();
    verifySerDeAndStandardMethods(instanceOfGeneratedClass);
  }

}
