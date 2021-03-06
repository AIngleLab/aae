/*

 */
package org.apache.trevni;

import java.io.IOException;

import java.util.Arrays;

class ColumnDescriptor<T extends Comparable> {
  final Input file;
  final ColumnMetaData metaData;

  long start;
  long dataStart;

  BlockDescriptor[] blocks;

  long[] blockStarts; // for random access
  long[] firstRows; // for binary searches
  T[] firstValues; // for binary searches

  public ColumnDescriptor(Input file, ColumnMetaData metaData) {
    this.file = file;
    this.metaData = metaData;
  }

  public int findBlock(long row) {
    int block = Arrays.binarySearch(firstRows, row);
    if (block < 0)
      block = -block - 2;
    return block;
  }

  public int findBlock(T value) {
    int block = Arrays.binarySearch(firstValues, value);
    if (block < 0)
      block = -block - 2;
    return block;
  }

  public int blockCount() {
    return blocks.length;
  }

  public long lastRow(int block) {
    if (blocks.length == 0 || block < 0)
      return 0;
    return firstRows[block] + blocks[block].rowCount;
  }

  public void ensureBlocksRead() throws IOException {
    if (blocks != null)
      return;

    // read block descriptors
    InputBuffer in = new InputBuffer(file, start);
    int blockCount = in.readFixed32();
    BlockDescriptor[] blocks = new BlockDescriptor[blockCount];
    if (metaData.hasIndexValues())
      firstValues = (T[]) new Comparable[blockCount];

    for (int i = 0; i < blockCount; i++) {
      blocks[i] = BlockDescriptor.read(in);
      if (metaData.hasIndexValues())
        firstValues[i] = in.readValue(metaData.getType());
    }
    dataStart = in.tell();

    // compute blockStarts and firstRows
    Checksum checksum = Checksum.get(metaData);
    blockStarts = new long[blocks.length];
    firstRows = new long[blocks.length];
    long startPosition = dataStart;
    long row = 0;
    for (int i = 0; i < blockCount; i++) {
      BlockDescriptor b = blocks[i];
      blockStarts[i] = startPosition;
      firstRows[i] = row;
      startPosition += b.compressedSize + checksum.size();
      row += b.rowCount;
    }
    this.blocks = blocks;
  }

}
