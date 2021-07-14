/*

 */
package org.apache.trevni;

import java.io.IOException;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import org.xerial.snappy.Snappy;

/** Implements <a href="https://code.google.com/p/snappy/">Snappy</a> codec. */
final class SnappyCodec extends Codec {

  @Override
  ByteBuffer compress(ByteBuffer in) throws IOException {
    int offset = computeOffset(in);
    ByteBuffer out = ByteBuffer.allocate(Snappy.maxCompressedLength(in.remaining()));
    int size = Snappy.compress(in.array(), offset, in.remaining(), out.array(), 0);
    ((Buffer) out).limit(size);
    return out;
  }

  @Override
  ByteBuffer decompress(ByteBuffer in) throws IOException {
    int offset = computeOffset(in);
    ByteBuffer out = ByteBuffer.allocate(Snappy.uncompressedLength(in.array(), offset, in.remaining()));
    int size = Snappy.uncompress(in.array(), offset, in.remaining(), out.array(), 0);
    ((Buffer) out).limit(size);
    return out;
  }

}
