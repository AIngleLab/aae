/*

 */

package org.apache.aingle.codegentest;

import org.apache.aingle.Conversion;
import org.apache.aingle.LogicalType;
import org.apache.aingle.LogicalTypes;
import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericFixed;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.util.Arrays;

public class CustomDecimalConversion extends Conversion<CustomDecimal> {
  @Override
  public Class<CustomDecimal> getConvertedType() {
    return CustomDecimal.class;
  }

  @Override
  public String getLogicalTypeName() {
    return "decimal";
  }

  @Override
  public CustomDecimal fromBytes(ByteBuffer value, Schema schema, LogicalType type) {
    int scale = ((LogicalTypes.Decimal) type).getScale();
    byte[] bytes = value.get(new byte[value.remaining()]).array();
    return new CustomDecimal(new BigInteger(bytes), scale);
  }

  @Override
  public ByteBuffer toBytes(CustomDecimal value, Schema schema, LogicalType type) {
    int scale = ((LogicalTypes.Decimal) type).getScale();
    return ByteBuffer.wrap(value.toByteArray(scale));
  }

  @Override
  public CustomDecimal fromFixed(GenericFixed value, Schema schema, LogicalType type) {
    int scale = ((LogicalTypes.Decimal) type).getScale();
    return new CustomDecimal(new BigInteger(value.bytes()), scale);
  }

  @Override
  public GenericFixed toFixed(CustomDecimal value, Schema schema, LogicalType type) {
    int scale = ((LogicalTypes.Decimal) type).getScale();
    byte fillByte = (byte) (value.signum() < 0 ? 0xFF : 0x00);
    byte[] unscaled = value.toByteArray(scale);
    byte[] bytes = new byte[schema.getFixedSize()];
    int offset = bytes.length - unscaled.length;

    // Fill the front of the array and copy remaining with unscaled values
    Arrays.fill(bytes, 0, offset, fillByte);
    System.arraycopy(unscaled, 0, bytes, offset, bytes.length - offset);

    return new GenericData.Fixed(schema, bytes);
  }
}
