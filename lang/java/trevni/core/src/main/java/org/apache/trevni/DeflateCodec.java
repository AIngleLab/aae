/*

 */
package org.apache.trevni;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;
import java.util.zip.InflaterOutputStream;

/** Implements DEFLATE (RFC1951) compression and decompression. */
class DeflateCodec extends Codec {
  private ByteArrayOutputStream outputBuffer;
  private Deflater deflater;
  private Inflater inflater;

  @Override
  ByteBuffer compress(ByteBuffer data) throws IOException {
    ByteArrayOutputStream baos = getOutputBuffer(data.remaining());
    try (OutputStream outputStream = new DeflaterOutputStream(baos, getDeflater())) {
      outputStream.write(data.array(), computeOffset(data), data.remaining());
    }
    return ByteBuffer.wrap(baos.toByteArray());
  }

  @Override
  ByteBuffer decompress(ByteBuffer data) throws IOException {
    ByteArrayOutputStream baos = getOutputBuffer(data.remaining());
    try (OutputStream outputStream = new InflaterOutputStream(baos, getInflater())) {
      outputStream.write(data.array(), computeOffset(data), data.remaining());
    }
    return ByteBuffer.wrap(baos.toByteArray());
  }

  private Inflater getInflater() {
    if (null == inflater)
      inflater = new Inflater(true);
    inflater.reset();
    return inflater;
  }

  private Deflater getDeflater() {
    if (null == deflater)
      deflater = new Deflater(Deflater.DEFAULT_COMPRESSION, true);
    deflater.reset();
    return deflater;
  }

  private ByteArrayOutputStream getOutputBuffer(int suggestedLength) {
    if (null == outputBuffer)
      outputBuffer = new ByteArrayOutputStream(suggestedLength);
    outputBuffer.reset();
    return outputBuffer;
  }

}
