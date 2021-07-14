/*

 */

package org.apache.aingle.io;

import org.junit.Assert;
import org.junit.Test;

public class TestBinaryData {

  /**
   * Insert a Long value into an Array. The worst-case scenario is
   * {@link Long#MAX_VALUE} because it requires 9 bytes to encode (instead of the
   * normal 8). When skipping it, the next byte should be 10.
   */
  @Test
  public void testSkipLong() {
    byte[] b = new byte[10];
    BinaryData.encodeLong(Long.MAX_VALUE, b, 0);

    final int nextIndex = BinaryData.skipLong(b, 0);

    Assert.assertEquals(nextIndex, 10);
  }

}
