/*

 */
package org.apache.aingle.ipc.stats;

import org.apache.aingle.ipc.stats.Stopwatch.Ticks;

/** Implements Ticks with manual time-winding. */
public class FakeTicks implements Ticks {
  long time = 0;

  @Override
  public long ticks() {
    return time;
  }

  public void passTime(long nanos) {
    time += nanos;
  }

}
