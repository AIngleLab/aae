/*

 */
package org.apache.aingle.ipc.stats;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

public class TestStopwatch {
  @Test
  public void testNormal() {
    FakeTicks f = new FakeTicks();
    Stopwatch s = new Stopwatch(f);
    f.passTime(10);
    s.start();
    f.passTime(20);
    assertEquals(20, s.elapsedNanos());
    f.passTime(40);
    s.stop();
    f.passTime(80);
    assertEquals(60, s.elapsedNanos());
  }

  @Test(expected = IllegalStateException.class)
  public void testNotStarted1() {
    FakeTicks f = new FakeTicks();
    Stopwatch s = new Stopwatch(f);
    s.elapsedNanos();
  }

  @Test(expected = IllegalStateException.class)
  public void testNotStarted2() {
    FakeTicks f = new FakeTicks();
    Stopwatch s = new Stopwatch(f);
    s.stop();
  }

  @Test(expected = IllegalStateException.class)
  public void testTwiceStarted() {
    FakeTicks f = new FakeTicks();
    Stopwatch s = new Stopwatch(f);
    s.start();
    s.start();
  }

  @Test(expected = IllegalStateException.class)
  public void testTwiceStopped() {
    FakeTicks f = new FakeTicks();
    Stopwatch s = new Stopwatch(f);
    s.start();
    s.stop();
    s.stop();
  }

  @Test
  public void testSystemStopwatch() {
    Stopwatch s = new Stopwatch(Stopwatch.SYSTEM_TICKS);
    s.start();
    s.stop();
    assertTrue(s.elapsedNanos() >= 0);
  }

}
