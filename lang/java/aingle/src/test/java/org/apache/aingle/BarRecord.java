/*

 */
package org.apache.aingle;

public class BarRecord {
  private String beerMsg;

  public BarRecord() {
  }

  public BarRecord(String beerMsg) {
    this.beerMsg = beerMsg;
  }

  @Override
  public boolean equals(Object that) {
    if (that instanceof BarRecord) {
      if (this.beerMsg == null) {
        return ((BarRecord) that).beerMsg == null;
      } else {
        return this.beerMsg.equals(((BarRecord) that).beerMsg);
      }
    }
    return false;
  }

  @Override
  public int hashCode() {
    return beerMsg.hashCode();
  }

  @Override
  public String toString() {
    return BarRecord.class.getSimpleName() + "{msg=" + beerMsg + "}";
  }
}
