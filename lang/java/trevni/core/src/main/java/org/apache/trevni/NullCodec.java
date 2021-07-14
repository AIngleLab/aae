/*

 */
package org.apache.trevni;

import java.io.IOException;
import java.nio.ByteBuffer;

/** Implements "null" (pass through) codec. */
final class NullCodec extends Codec {

  @Override
  ByteBuffer compress(ByteBuffer buffer) throws IOException {
    return buffer;
  }

  @Override
  ByteBuffer decompress(ByteBuffer data) throws IOException {
    return data;
  }

}
