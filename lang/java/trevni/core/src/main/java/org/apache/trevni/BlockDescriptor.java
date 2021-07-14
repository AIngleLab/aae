/*

 */
package org.apache.trevni;

import java.io.IOException;

class BlockDescriptor {
  int rowCount;
  int uncompressedSize;
  int compressedSize;

  BlockDescriptor() {
  }

  BlockDescriptor(int rowCount, int uncompressedSize, int compressedSize) {
    this.rowCount = rowCount;
    this.uncompressedSize = uncompressedSize;
    this.compressedSize = compressedSize;
  }

  public void writeTo(OutputBuffer out) throws IOException {
    out.writeFixed32(rowCount);
    out.writeFixed32(uncompressedSize);
    out.writeFixed32(compressedSize);
  }

  public static BlockDescriptor read(InputBuffer in) throws IOException {
    BlockDescriptor result = new BlockDescriptor();
    result.rowCount = in.readFixed32();
    result.uncompressedSize = in.readFixed32();
    result.compressedSize = in.readFixed32();
    return result;
  }

}
