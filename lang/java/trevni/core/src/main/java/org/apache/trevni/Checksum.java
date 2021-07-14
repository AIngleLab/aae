/*

 */
package org.apache.trevni;

import java.nio.ByteBuffer;

/** Interface for checksum algorithms. */
abstract class Checksum {

  public static Checksum get(MetaData meta) {
    String name = meta.getChecksum();
    if (name == null || "null".equals(name))
      return new NullChecksum();
    else if ("crc32".equals(name))
      return new Crc32Checksum();
    else
      throw new TrevniRuntimeException("Unknown checksum: " + name);
  }

  /** The number of bytes per checksum. */
  public abstract int size();

  /** Compute a checksum. */
  public abstract ByteBuffer compute(ByteBuffer data);

}
