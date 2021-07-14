/*

 */
package org.apache.aingle.specific;

import java.io.Externalizable;
import java.io.ObjectOutput;
import java.io.ObjectInput;
import java.io.IOException;
import java.util.Arrays;
import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericFixed;
import org.apache.aingle.io.BinaryData;

/** Base class for generated fixed-sized data classes. */
public abstract class SpecificFixed implements GenericFixed, Comparable<SpecificFixed>, Externalizable {

  private byte[] bytes;

  public SpecificFixed() {
    bytes(new byte[getSchema().getFixedSize()]);
  }

  public SpecificFixed(byte[] bytes) {
    bytes(bytes);
  }

  public void bytes(byte[] bytes) {
    this.bytes = bytes;
  }

  @Override
  public byte[] bytes() {
    return bytes;
  }

  @Override
  public abstract Schema getSchema();

  @Override
  public boolean equals(Object o) {
    if (o == this)
      return true;
    return o instanceof GenericFixed && Arrays.equals(bytes, ((GenericFixed) o).bytes());
  }

  @Override
  public int hashCode() {
    return Arrays.hashCode(bytes);
  }

  @Override
  public String toString() {
    return Arrays.toString(bytes);
  }

  @Override
  public int compareTo(SpecificFixed that) {
    return BinaryData.compareBytes(this.bytes, 0, this.bytes.length, that.bytes, 0, that.bytes.length);
  }

  @Override
  public abstract void writeExternal(ObjectOutput out) throws IOException;

  @Override
  public abstract void readExternal(ObjectInput in) throws IOException;

}
