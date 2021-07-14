/*

 */
package org.apache.aingle.specific;

import org.apache.aingle.test.errors.TestError;
import org.junit.Assert;
import org.junit.Test;

/**
 * Unit test for the SpecificErrorBuilderBase class.
 */
public class TestSpecificErrorBuilder {
  @Test
  public void testSpecificErrorBuilder() {
    TestError.Builder testErrorBuilder = TestError.newBuilder().setValue("value").setCause(new NullPointerException())
        .setMessage$("message$");

    // Test has methods
    Assert.assertTrue(testErrorBuilder.hasValue());
    Assert.assertNotNull(testErrorBuilder.getValue());
    Assert.assertTrue(testErrorBuilder.hasCause());
    Assert.assertNotNull(testErrorBuilder.getCause());
    Assert.assertTrue(testErrorBuilder.hasMessage$());
    Assert.assertNotNull(testErrorBuilder.getMessage$());

    TestError testError = testErrorBuilder.build();
    Assert.assertEquals("value", testError.getValue());
    Assert.assertEquals("value", testError.getMessage());
    Assert.assertEquals("message$", testError.getMessage$());

    // Test copy constructor
    Assert.assertEquals(testErrorBuilder, TestError.newBuilder(testErrorBuilder));
    Assert.assertEquals(testErrorBuilder, TestError.newBuilder(testError));

    TestError error = new TestError("value", new NullPointerException());
    error.setMessage$("message");
    Assert.assertEquals(error,
        TestError.newBuilder().setValue("value").setCause(new NullPointerException()).setMessage$("message").build());

    // Test clear
    testErrorBuilder.clearValue();
    Assert.assertFalse(testErrorBuilder.hasValue());
    Assert.assertNull(testErrorBuilder.getValue());
    testErrorBuilder.clearCause();
    Assert.assertFalse(testErrorBuilder.hasCause());
    Assert.assertNull(testErrorBuilder.getCause());
    testErrorBuilder.clearMessage$();
    Assert.assertFalse(testErrorBuilder.hasMessage$());
    Assert.assertNull(testErrorBuilder.getMessage$());
  }

  @Test(expected = org.apache.aingle.AIngleRuntimeException.class)
  public void attemptToSetNonNullableFieldToNull() {
    TestError.newBuilder().setMessage$(null);
  }
}
