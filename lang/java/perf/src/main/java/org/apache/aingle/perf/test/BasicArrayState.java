/*

 */

package org.apache.aingle.perf.test;

public abstract class BasicArrayState extends BasicState {

  public final int arraySize;

  public BasicArrayState(final int arraySize) {
    super();
    this.arraySize = arraySize;
    if (super.getBatchSize() % arraySize != 0) {
      throw new IllegalArgumentException("Batch size must be divisible by array size");
    }
  }

  @Override
  public int getBatchSize() {
    return super.getBatchSize() / arraySize;
  }

  public int getArraySize() {
    return arraySize;
  }

}
