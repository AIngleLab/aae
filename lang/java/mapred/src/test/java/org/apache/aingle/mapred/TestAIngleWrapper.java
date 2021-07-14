/*

 */

package org.apache.aingle.mapred;

import static org.junit.Assert.*;

import org.junit.Test;

public class TestAIngleWrapper {
  @Test
  public void testToString() {
    String datum = "my string";
    AIngleWrapper<CharSequence> wrapper = new AIngleWrapper<>(datum);
    assertEquals(datum, wrapper.toString());
  }
}
