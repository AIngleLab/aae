/*

 */
package org.apache.aingle.file;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import org.apache.commons.compress.compressors.xz.XZCompressorInputStream;
import org.apache.commons.compress.compressors.xz.XZCompressorOutputStream;
import org.apache.commons.compress.utils.IOUtils;

/** * Implements xz compression and decompression. */
public class XZCodec extends Codec {
  public final static int DEFAULT_COMPRESSION = 6;

  static class Option extends CodecFactory {
    private int compressionLevel;

    Option(int compressionLevel) {
      this.compressionLevel = compressionLevel;
    }

    @Override
    protected Codec createInstance() {
      return new XZCodec(compressionLevel);
    }
  }

  private ByteArrayOutputStream outputBuffer;
  private int compressionLevel;

  public XZCodec(int compressionLevel) {
    this.compressionLevel = compressionLevel;
  }

  @Override
  public String getName() {
    return DataFileConstants.XZ_CODEC;
  }

  @Override
  public ByteBuffer compress(ByteBuffer data) throws IOException {
    ByteArrayOutputStream baos = getOutputBuffer(data.remaining());
    try (OutputStream outputStream = new XZCompressorOutputStream(baos, compressionLevel)) {
      outputStream.write(data.array(), computeOffset(data), data.remaining());
    }
    return ByteBuffer.wrap(baos.toByteArray());
  }

  @Override
  public ByteBuffer decompress(ByteBuffer data) throws IOException {
    ByteArrayOutputStream baos = getOutputBuffer(data.remaining());
    InputStream bytesIn = new ByteArrayInputStream(data.array(), computeOffset(data), data.remaining());

    try (InputStream ios = new XZCompressorInputStream(bytesIn)) {
      IOUtils.copy(ios, baos);
    }
    return ByteBuffer.wrap(baos.toByteArray());
  }

  // get and initialize the output buffer for use.
  private ByteArrayOutputStream getOutputBuffer(int suggestedLength) {
    if (null == outputBuffer) {
      outputBuffer = new ByteArrayOutputStream(suggestedLength);
    }
    outputBuffer.reset();
    return outputBuffer;
  }

  @Override
  public int hashCode() {
    return compressionLevel;
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj)
      return true;
    if (obj == null || obj.getClass() != getClass())
      return false;
    XZCodec other = (XZCodec) obj;
    return (this.compressionLevel == other.compressionLevel);
  }

  @Override
  public String toString() {
    return getName() + "-" + compressionLevel;
  }
}
