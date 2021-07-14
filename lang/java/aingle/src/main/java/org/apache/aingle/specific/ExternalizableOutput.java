/*

 */

package org.apache.aingle.specific;

import java.io.OutputStream;
import java.io.IOException;
import java.io.ObjectOutput;

/**
 * Helper to permit Externalizable implementations that write to an
 * OutputStream.
 */
class ExternalizableOutput extends OutputStream {
  private final ObjectOutput out;

  public ExternalizableOutput(ObjectOutput out) {
    this.out = out;
  }

  @Override
  public void flush() throws IOException {
    out.flush();
  }

  @Override
  public void close() throws IOException {
    out.close();
  }

  @Override
  public void write(int c) throws IOException {
    out.write(c);
  }

  @Override
  public void write(byte[] b) throws IOException {
    out.write(b);
  }

  @Override
  public void write(byte[] b, int offset, int len) throws IOException {
    out.write(b, offset, len);
  }
}
