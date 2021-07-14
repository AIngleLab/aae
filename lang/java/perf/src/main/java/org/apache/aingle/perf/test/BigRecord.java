/*

 */

package org.apache.aingle.perf.test;

import java.util.Random;

public class BigRecord {
  public double d1;
  public double d11;
  public float f2;
  public float f22;
  public int f3;
  public int f33;
  public long f4;
  public long f44;
  public byte f5;
  public byte f55;
  public short f6;
  public short f66;

  public BigRecord() {
  }

  public BigRecord(final Random r) {
    this.d1 = r.nextDouble();
    this.d11 = r.nextDouble();
    this.f2 = r.nextFloat();
    this.f22 = r.nextFloat();
    this.f3 = r.nextInt();
    this.f33 = r.nextInt();
    this.f4 = r.nextLong();
    this.f44 = r.nextLong();
    this.f5 = (byte) r.nextInt();
    this.f55 = (byte) r.nextInt();
    this.f6 = (short) r.nextInt();
    this.f66 = (short) r.nextInt();
  }
}
