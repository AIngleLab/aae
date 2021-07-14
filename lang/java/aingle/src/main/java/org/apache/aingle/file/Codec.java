/*

 */
package org.apache.aingle.file;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Interface for AIngle-supported compression codecs for data files.
 *
 * Note that Codec objects may maintain internal state (e.g. buffers) and are
 * not thread safe.
 */
public abstract class Codec {
  /** Name of the codec; written to the file's metadata. */
  public abstract String getName();

  /** Compresses the input data */
  public abstract ByteBuffer compress(ByteBuffer uncompressedData) throws IOException;

  /** Decompress the data */
  public abstract ByteBuffer decompress(ByteBuffer compressedData) throws IOException;

  /**
   * Codecs must implement an equals() method. Two codecs, A and B are equal if:
   * the result of A and B decompressing content compressed by A is the same AND
   * the result of A and B decompressing content compressed by B is the same
   **/
  @Override
  public abstract boolean equals(Object other);

  /**
   * Codecs must implement a hashCode() method that is consistent with equals().
   */
  @Override
  public abstract int hashCode();

  @Override
  public String toString() {
    return getName();
  }

  // Codecs often reference the array inside a ByteBuffer. Compute the offset
  // to the start of data correctly in the case that our ByteBuffer
  // is a slice() of another.
  protected static int computeOffset(ByteBuffer data) {
    return data.arrayOffset() + data.position();
  }
}
