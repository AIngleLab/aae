/*

 */
package org.apache.aingle.file;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import org.apache.commons.compress.compressors.bzip2.BZip2CompressorInputStream;
import org.apache.commons.compress.compressors.bzip2.BZip2CompressorOutputStream;

/** * Implements bzip2 compression and decompression. */
public class BZip2Codec extends Codec {

  public static final int DEFAULT_BUFFER_SIZE = 64 * 1024;
  private final byte[] buffer = new byte[DEFAULT_BUFFER_SIZE];

  private ByteArrayOutputStream outputBuffer;

  static class Option extends CodecFactory {
    @Override
    protected Codec createInstance() {
      return new BZip2Codec();
    }
  }

  @Override
  public String getName() {
    return DataFileConstants.BZIP2_CODEC;
  }

  @Override
  public ByteBuffer compress(ByteBuffer uncompressedData) throws IOException {
    ByteArrayOutputStream baos = getOutputBuffer(uncompressedData.remaining());

    try (BZip2CompressorOutputStream outputStream = new BZip2CompressorOutputStream(baos)) {
      outputStream.write(uncompressedData.array(), computeOffset(uncompressedData), uncompressedData.remaining());
    }

    return ByteBuffer.wrap(baos.toByteArray());
  }

  @Override
  public ByteBuffer decompress(ByteBuffer compressedData) throws IOException {
    ByteArrayInputStream bais = new ByteArrayInputStream(compressedData.array(), computeOffset(compressedData),
        compressedData.remaining());
    try (BZip2CompressorInputStream inputStream = new BZip2CompressorInputStream(bais)) {
      ByteArrayOutputStream baos = new ByteArrayOutputStream();

      int readCount = -1;
      while ((readCount = inputStream.read(buffer, compressedData.position(), buffer.length)) > 0) {
        baos.write(buffer, 0, readCount);
      }

      return ByteBuffer.wrap(baos.toByteArray());
    }
  }

  @Override
  public int hashCode() {
    return getName().hashCode();
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj)
      return true;
    return obj != null && obj.getClass() == getClass();
  }

  // get and initialize the output buffer for use.
  private ByteArrayOutputStream getOutputBuffer(int suggestedLength) {
    if (null == outputBuffer) {
      outputBuffer = new ByteArrayOutputStream(suggestedLength);
    }
    outputBuffer.reset();
    return outputBuffer;
  }
}
