/*

 */

package org.apache.aingle.grpc;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import io.grpc.Drainable;

/**
 * An {@link InputStream} backed by AIngle RPC request/response that can drained
 * to a{@link OutputStream}.
 */
public abstract class AIngleInputStream extends InputStream implements Drainable {
  /**
   * Container to hold the serialized AIngle payload when its read before draining
   * it.
   */
  private ByteArrayInputStream partial;

  @Override
  public int read(byte[] b, int off, int len) throws IOException {
    return getPartialInternal().read(b, off, len);
  }

  @Override
  public int read() throws IOException {
    return getPartialInternal().read();
  }

  private ByteArrayInputStream getPartialInternal() throws IOException {
    if (partial == null) {
      ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
      drainTo(outputStream);
      partial = new ByteArrayInputStream(outputStream.toByteArray());
    }
    return partial;
  }

  protected ByteArrayInputStream getPartial() {
    return partial;
  }

  /**
   * An {@link OutputStream} that writes to a target {@link OutputStream} and
   * provides total number of bytes written to it.
   */
  protected static class CountingOutputStream extends OutputStream {
    private final OutputStream target;
    private int writtenCount = 0;

    public CountingOutputStream(OutputStream target) {
      this.target = target;
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
      target.write(b, off, len);
      writtenCount += len;
    }

    @Override
    public void write(int b) throws IOException {
      target.write(b);
      writtenCount += 1;
    }

    @Override
    public void flush() throws IOException {
      target.flush();
    }

    @Override
    public void close() throws IOException {
      target.close();
    }

    public int getWrittenCount() {
      return writtenCount;
    }
  }
}
