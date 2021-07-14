/*

 */
package org.apache.trevni;

import java.io.IOException;
import java.io.Closeable;

/** A byte source that supports positioned read and length. */
public interface Input extends Closeable {
  /** Return the total length of the input. */
  long length() throws IOException;

  /** Positioned read. */
  int read(long position, byte[] b, int start, int len) throws IOException;
}
