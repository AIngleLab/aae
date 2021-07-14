/*

 */
package org.apache.trevni;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.zip.CRC32;

/** Implements CRC32 checksum. */
final class Crc32Checksum extends Checksum {
  private CRC32 crc32 = new CRC32();

  @Override
  public int size() {
    return 4;
  }

  @Override
  public ByteBuffer compute(ByteBuffer data) {
    crc32.reset();
    crc32.update(data.array(), data.position(), data.remaining());

    ByteBuffer result = ByteBuffer.allocate(size());
    result.putInt((int) crc32.getValue());
    ((Buffer) result).flip();
    return result;
  }

}
