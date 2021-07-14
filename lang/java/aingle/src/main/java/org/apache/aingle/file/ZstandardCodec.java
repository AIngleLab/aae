/*

 */
package org.apache.aingle.file;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import org.apache.commons.compress.utils.IOUtils;

public class ZstandardCodec extends Codec {
  public final static int DEFAULT_COMPRESSION = 3;
  public final static boolean DEFAULT_USE_BUFFERPOOL = false;

  static class Option extends CodecFactory {
    private final int compressionLevel;
    private final boolean useChecksum;
    private final boolean useBufferPool;

    Option(int compressionLevel, boolean useChecksum, boolean useBufferPool) {
      this.compressionLevel = compressionLevel;
      this.useChecksum = useChecksum;
      this.useBufferPool = useBufferPool;
    }

    @Override
    protected Codec createInstance() {
      return new ZstandardCodec(compressionLevel, useChecksum, useBufferPool);
    }
  }

  private final int compressionLevel;
  private final boolean useChecksum;
  private final boolean useBufferPool;
  private ByteArrayOutputStream outputBuffer;

  /**
   * Create a ZstandardCodec instance with the given compressionLevel, checksum,
   * and bufferPool option
   **/
  public ZstandardCodec(int compressionLevel, boolean useChecksum, boolean useBufferPool) {
    this.compressionLevel = compressionLevel;
    this.useChecksum = useChecksum;
    this.useBufferPool = useBufferPool;
  }

  @Override
  public String getName() {
    return DataFileConstants.ZSTANDARD_CODEC;
  }

  @Override
  public ByteBuffer compress(ByteBuffer data) throws IOException {
    ByteArrayOutputStream baos = getOutputBuffer(data.remaining());
    try (OutputStream outputStream = ZstandardLoader.output(baos, compressionLevel, useChecksum, useBufferPool)) {
      outputStream.write(data.array(), computeOffset(data), data.remaining());
    }
    return ByteBuffer.wrap(baos.toByteArray());
  }

  @Override
  public ByteBuffer decompress(ByteBuffer compressedData) throws IOException {
    ByteArrayOutputStream baos = getOutputBuffer(compressedData.remaining());
    InputStream bytesIn = new ByteArrayInputStream(compressedData.array(), computeOffset(compressedData),
        compressedData.remaining());
    try (InputStream ios = ZstandardLoader.input(bytesIn, useBufferPool)) {
      IOUtils.copy(ios, baos);
    }
    return ByteBuffer.wrap(baos.toByteArray());
  }

  // get and initialize the output buffer for use.
  private ByteArrayOutputStream getOutputBuffer(int suggestedLength) {
    if (outputBuffer == null) {
      outputBuffer = new ByteArrayOutputStream(suggestedLength);
    }
    outputBuffer.reset();
    return outputBuffer;
  }

  @Override
  public int hashCode() {
    return getName().hashCode();
  }

  @Override
  public boolean equals(Object obj) {
    return (this == obj) || (obj != null && obj.getClass() == this.getClass());
  }

  @Override
  public String toString() {
    return getName() + "[" + compressionLevel + "]";
  }
}
