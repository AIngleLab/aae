/*

 */
package org.apache.aingle.ipc.stats;

/**
 * Specific implementation of histogram for integers, which also keeps track of
 * basic summary statistics.
 * 
 * @param <B>
 */
class IntegerHistogram<B> extends Histogram<B, Integer> {
  private float runningSum;
  private float runningSumOfSquares;

  public IntegerHistogram(Segmenter<B, Integer> segmenter) {
    super(segmenter);
  }

  @Override
  public void add(Integer value) {
    super.add(value);
    runningSum += value;
    runningSumOfSquares += value * value;
  }

  public float getMean() {
    if (totalCount == 0) {
      return -1;
    }
    return runningSum / (float) totalCount;
  }

  public float getUnbiasedStdDev() {
    if (totalCount <= 1) {
      return -1;
    }
    float mean = getMean();
    return (float) Math.sqrt((runningSumOfSquares - totalCount * mean * mean) / (float) (totalCount - 1));
  }
}
