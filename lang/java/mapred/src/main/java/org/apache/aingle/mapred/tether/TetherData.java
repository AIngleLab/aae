/*

 */

package org.apache.aingle.mapred.tether;

import java.nio.ByteBuffer;

/** A wrapper for a ByteBuffer containing binary-encoded data. */
class TetherData {
  private int count = 1; // only used for task input
  private ByteBuffer buffer;

  public TetherData() {
  }

  public TetherData(ByteBuffer buffer) {
    this.buffer = buffer;
  }

  /** Return the count of records in the buffer. Used for task input only. */
  public int count() {
    return count;
  }

  /** Set the count of records in the buffer. Used for task input only. */
  public void count(int count) {
    this.count = count;
  }

  /** Return the buffer. */
  public ByteBuffer buffer() {
    return buffer;
  }

  /** Set the buffer. */
  public void buffer(ByteBuffer buffer) {
    this.buffer = buffer;
  }
}
