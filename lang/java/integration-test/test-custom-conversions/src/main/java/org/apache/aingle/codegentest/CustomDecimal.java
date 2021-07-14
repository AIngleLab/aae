/*

 */

package org.apache.aingle.codegentest;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.math.RoundingMode;

/**
 * Wraps a BigDecimal just to demonstrate that it is possible to use custom
 * implementation classes with custom conversions.
 */
public class CustomDecimal implements Comparable<CustomDecimal> {

  private final BigDecimal internalValue;

  public CustomDecimal(BigInteger value, int scale) {
    internalValue = new BigDecimal(value, scale);
  }

  public byte[] toByteArray(int scale) {
    final BigDecimal correctlyScaledValue;
    if (scale != internalValue.scale()) {
      correctlyScaledValue = internalValue.setScale(scale, RoundingMode.HALF_UP);
    } else {
      correctlyScaledValue = internalValue;
    }
    return correctlyScaledValue.unscaledValue().toByteArray();

  }

  int signum() {
    return internalValue.signum();
  }

  @Override
  public boolean equals(Object o) {
    if (this == o)
      return true;
    if (o == null || getClass() != o.getClass())
      return false;

    CustomDecimal that = (CustomDecimal) o;

    return internalValue.equals(that.internalValue);
  }

  @Override
  public int hashCode() {
    return internalValue.hashCode();
  }

  @Override
  public int compareTo(CustomDecimal o) {
    return this.internalValue.compareTo(o.internalValue);
  }
}
