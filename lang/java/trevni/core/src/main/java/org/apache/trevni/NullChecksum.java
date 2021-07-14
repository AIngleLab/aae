/*

 */
package org.apache.trevni;

import java.nio.ByteBuffer;

/** Implements "null" (empty) checksum. */
final class NullChecksum extends Checksum {

  @Override
  public int size() {
    return 0;
  }

  @Override
  public ByteBuffer compute(ByteBuffer data) {
    return ByteBuffer.allocate(0);
  }

}
