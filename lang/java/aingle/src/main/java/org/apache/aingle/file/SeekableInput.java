/*

 */
package org.apache.aingle.file;

import java.io.IOException;
import java.io.Closeable;

/** An InputStream that supports seek and tell. */
public interface SeekableInput extends Closeable {

  /**
   * Set the position for the next {@link java.io.InputStream#read(byte[],int,int)
   * read()}.
   */
  void seek(long p) throws IOException;

  /**
   * Return the position of the next
   * {@link java.io.InputStream#read(byte[],int,int) read()}.
   */
  long tell() throws IOException;

  /** Return the length of the file. */
  long length() throws IOException;

  /** Equivalent to {@link java.io.InputStream#read(byte[],int,int)}. */
  int read(byte[] b, int off, int len) throws IOException;
}
