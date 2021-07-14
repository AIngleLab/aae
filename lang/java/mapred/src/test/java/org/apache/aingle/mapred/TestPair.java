/*

 */

package org.apache.aingle.mapred;

import java.util.ArrayList;

import org.apache.aingle.AIngleRuntimeException;

import org.junit.Test;
import static org.junit.Assert.*;

public class TestPair {

  @Test
  public void testCollectionFailure() throws Exception {
    try {
      new Pair("foo", new ArrayList());
    } catch (AIngleRuntimeException e) {
      assertTrue(e.getMessage().startsWith("Cannot infer schema"));
      return;
    }
    fail("Expected an AIngleRuntimeException");
  }

}
