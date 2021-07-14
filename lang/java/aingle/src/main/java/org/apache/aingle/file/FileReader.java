/*

 */
package org.apache.aingle.file;

import java.io.IOException;
import java.io.Closeable;
import java.util.Iterator;
import java.util.NoSuchElementException;

import org.apache.aingle.Schema;

/** Interface for reading data from a file. */
public interface FileReader<D> extends Iterator<D>, Iterable<D>, Closeable {
  /** Return the schema for data in this file. */
  Schema getSchema();

  /**
   * Read the next datum from the file.
   * 
   * @param reuse an instance to reuse.
   * @throws NoSuchElementException if no more remain in the file.
   */
  D next(D reuse) throws IOException;

  /**
   * Move to the next synchronization point after a position. To process a range
   * of file entires, call this with the starting position, then check
   * {@link #pastSync(long)} with the end point before each call to
   * {@link #next()}.
   */
  void sync(long position) throws IOException;

  /** Return true if past the next synchronization point after a position. */
  boolean pastSync(long position) throws IOException;

  /** Return the current position in the input. */
  long tell() throws IOException;

}
