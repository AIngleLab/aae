/*

 */
package org.apache.aingle.ipc.stats;

/** Encapsulates the passing of time. */
class Stopwatch {
  /** Encapsulates ticking time sources. */
  interface Ticks {
    /**
     * Returns a number of "ticks" in nanoseconds. This should be monotonically
     * non-decreasing.
     */
    long ticks();
  }

  /** Default System time source. */
  public final static Ticks SYSTEM_TICKS = new SystemTicks();

  private Ticks ticks;
  private long start;
  private long elapsed = -1;
  private boolean running;

  public Stopwatch(Ticks ticks) {
    this.ticks = ticks;
  }

  /** Returns seconds that have elapsed since start() */
  public long elapsedNanos() {
    if (running) {
      return this.ticks.ticks() - start;
    } else {
      if (elapsed == -1)
        throw new IllegalStateException();
      return elapsed;
    }
  }

  /** Starts the stopwatch. */
  public void start() {
    if (running)
      throw new IllegalStateException();
    start = ticks.ticks();
    running = true;
  }

  /** Stops the stopwatch and calculates the elapsed time. */
  public void stop() {
    if (!running)
      throw new IllegalStateException();
    elapsed = ticks.ticks() - start;
    running = false;
  }

  /** Implementation of Ticks using System.nanoTime(). */
  private static class SystemTicks implements Ticks {
    @Override
    public long ticks() {
      return System.nanoTime();
    }
  }
}
