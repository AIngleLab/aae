/*

 */

package org.apache.aingle.specific;

import java.io.InputStream;
import java.io.IOException;
import java.io.ObjectInput;

/**
 * Helper to permit Externalizable implementations that write to an InputStream.
 */
class ExternalizableInput extends InputStream {
  private final ObjectInput in;

  public ExternalizableInput(ObjectInput in) {
    this.in = in;
  }

  @Override
  public int available() throws IOException {
    return in.available();
  }

  @Override
  public void close() throws IOException {
    in.close();
  }

  @Override
  public boolean markSupported() {
    return false;
  }

  @Override
  public int read() throws IOException {
    return in.read();
  }

  @Override
  public int read(byte[] b) throws IOException {
    return in.read(b);
  }

  @Override
  public int read(byte[] b, int offset, int len) throws IOException {
    return in.read(b, offset, len);
  }

  @Override
  public long skip(long n) throws IOException {
    return in.skip(n);
  }
}
