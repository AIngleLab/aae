/*

 */
package org.apache.aingle.file;

import java.io.IOException;
import java.nio.ByteBuffer;

/** Implements "null" (pass through) codec. */
final class NullCodec extends Codec {

  private static final NullCodec INSTANCE = new NullCodec();

  static class Option extends CodecFactory {
    @Override
    protected Codec createInstance() {
      return INSTANCE;
    }
  }

  /** No options available for NullCodec. */
  public static final CodecFactory OPTION = new Option();

  @Override
  public String getName() {
    return DataFileConstants.NULL_CODEC;
  }

  @Override
  public ByteBuffer compress(ByteBuffer buffer) throws IOException {
    return buffer;
  }

  @Override
  public ByteBuffer decompress(ByteBuffer data) throws IOException {
    return data;
  }

  @Override
  public boolean equals(Object other) {
    if (this == other)
      return true;
    return (other != null && other.getClass() == getClass());
  }

  @Override
  public int hashCode() {
    return 2;
  }
}
