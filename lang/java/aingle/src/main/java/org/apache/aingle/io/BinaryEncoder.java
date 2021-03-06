/*

 */
package org.apache.aingle.io;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

import org.apache.aingle.util.Utf8;

/**
 * An abstract {@link Encoder} for AIngle's binary encoding.
 * <p/>
 * To construct and configure instances, use {@link EncoderFactory}
 *
 * @see EncoderFactory
 * @see BufferedBinaryEncoder
 * @see DirectBinaryEncoder
 * @see BlockingBinaryEncoder
 * @see Encoder
 * @see Decoder
 */
public abstract class BinaryEncoder extends Encoder {

  @Override
  public void writeNull() throws IOException {
  }

  @Override
  public void writeString(Utf8 utf8) throws IOException {
    this.writeBytes(utf8.getBytes(), 0, utf8.getByteLength());
  }

  @Override
  public void writeString(String string) throws IOException {
    if (0 == string.length()) {
      writeZero();
      return;
    }
    byte[] bytes = string.getBytes(StandardCharsets.UTF_8);
    writeInt(bytes.length);
    writeFixed(bytes, 0, bytes.length);
  }

  @Override
  public void writeBytes(ByteBuffer bytes) throws IOException {
    int len = bytes.limit() - bytes.position();
    if (0 == len) {
      writeZero();
    } else {
      writeInt(len);
      writeFixed(bytes);
    }
  }

  @Override
  public void writeBytes(byte[] bytes, int start, int len) throws IOException {
    if (0 == len) {
      writeZero();
      return;
    }
    this.writeInt(len);
    this.writeFixed(bytes, start, len);
  }

  @Override
  public void writeEnum(int e) throws IOException {
    this.writeInt(e);
  }

  @Override
  public void writeArrayStart() throws IOException {
  }

  @Override
  public void setItemCount(long itemCount) throws IOException {
    if (itemCount > 0) {
      this.writeLong(itemCount);
    }
  }

  @Override
  public void startItem() throws IOException {
  }

  @Override
  public void writeArrayEnd() throws IOException {
    writeZero();
  }

  @Override
  public void writeMapStart() throws IOException {
  }

  @Override
  public void writeMapEnd() throws IOException {
    writeZero();
  }

  @Override
  public void writeIndex(int unionIndex) throws IOException {
    writeInt(unionIndex);
  }

  /** Write a zero byte to the underlying output. **/
  protected abstract void writeZero() throws IOException;

  /**
   * Returns the number of bytes currently buffered by this encoder. If this
   * Encoder does not buffer, this will always return zero.
   * <p/>
   * Call {@link #flush()} to empty the buffer to the underlying output.
   */
  public abstract int bytesBuffered();

}
