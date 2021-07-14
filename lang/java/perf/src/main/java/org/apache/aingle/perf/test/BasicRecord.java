/*

 */

package org.apache.aingle.perf.test;

import java.util.Random;

public final class BasicRecord {
  public double f1;
  public double f2;
  public double f3;
  public int f4;
  public int f5;
  public int f6;

  public BasicRecord() {

  }

  public BasicRecord(final Random r) {
    f1 = r.nextDouble();
    f2 = r.nextDouble();
    f3 = r.nextDouble();
    f4 = r.nextInt();
    f5 = r.nextInt();
    f6 = r.nextInt();
  }
}
