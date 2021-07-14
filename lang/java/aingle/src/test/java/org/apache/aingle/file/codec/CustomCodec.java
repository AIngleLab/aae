/*

 */

package org.apache.aingle.file.codec;

import static java.nio.charset.StandardCharsets.UTF_8;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.apache.aingle.file.Codec;

/**
 * Simple Custom Codec to validate making Codec Public Compress and Decompress
 * operations are just bitwise-NOT of data
 */
public class CustomCodec extends Codec {

  private static final String CODECNAME = "CUSTOMCODEC";

  @Override
  public String getName() {
    return CODECNAME;
  }

  @Override
  public ByteBuffer compress(ByteBuffer in) throws IOException {
    ByteBuffer out = ByteBuffer.allocate(in.remaining());
    while (in.position() < in.capacity())
      out.put((byte) ~in.get());
    return out;
  }

  @Override
  public ByteBuffer decompress(ByteBuffer in) throws IOException {
    ByteBuffer out = ByteBuffer.allocate(in.remaining());
    while (in.position() < in.capacity())
      out.put((byte) ~in.get());
    return out;
  }

  @Override
  public boolean equals(Object other) {
    if (this == other)
      return true;
    if (other instanceof Codec) {
      ByteBuffer original = ByteBuffer.allocate(getName().getBytes(UTF_8).length);
      original.put(getName().getBytes(UTF_8));
      original.rewind();
      try {
        return compareDecompress((Codec) other, original);
      } catch (IOException e) {
        return false;
      }
    } else
      return false;
  }

  /**
   * Codecs must implement an equals() method. Two codecs, A and B are equal if:
   * the result of A and B decompressing content compressed by A is the same AND
   * the result of A and B decompressing content compressed by B is the same
   */
  private boolean compareDecompress(Codec other, ByteBuffer original) throws IOException {
    ByteBuffer compressedA = this.compress(original);
    original.rewind();
    ByteBuffer compressedB = other.compress(original);

    return this.decompress(compressedA).equals(other.decompress((ByteBuffer) compressedA.rewind()))
        && this.decompress(compressedB).equals(other.decompress((ByteBuffer) compressedB.rewind()));
  }

  @Override
  public int hashCode() {
    return getName().hashCode();
  }
}
