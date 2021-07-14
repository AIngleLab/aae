/*

 */
package org.apache.aingle.ipc.stats;

/**
 * Specific implementation of histogram for floats, which also keeps track of
 * basic summary statistics.
 * 
 * @param <B>
 */
class FloatHistogram<B> extends Histogram<B, Float> {
  private float runningSum;
  private float runningSumOfSquares;

  public FloatHistogram(Segmenter<B, Float> segmenter) {
    super(segmenter);
  }

  @Override
  public void add(Float value) {
    super.add(value);
    runningSum += value;
    runningSumOfSquares += value * value;
  }

  public float getMean() {
    if (totalCount == 0) {
      return Float.NaN;
    }
    return runningSum / totalCount;
  }

  public float getUnbiasedStdDev() {
    if (totalCount <= 1) {
      return Float.NaN;
    }
    float mean = getMean();
    return (float) Math.sqrt((runningSumOfSquares - totalCount * mean * mean) / (totalCount - 1));
  }
}
