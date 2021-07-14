/*

 */
package org.apache.aingle.specific;

import java.io.Externalizable;
import java.io.ObjectOutput;
import java.io.ObjectInput;
import java.io.IOException;
import org.apache.aingle.AIngleRuntimeException;

import org.apache.aingle.Conversion;
import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.io.ResolvingDecoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.message.MessageDecoder;
import org.apache.aingle.message.MessageEncoder;

/** Base class for generated record classes. */
public abstract class SpecificRecordBase
    implements SpecificRecord, Comparable<SpecificRecord>, GenericRecord, Externalizable {

  @Override
  public abstract Schema getSchema();

  @Override
  public abstract Object get(int field);

  @Override
  public abstract void put(int field, Object value);

  public SpecificData getSpecificData() {
    // Default implementation for backwards compatibility, overridden in generated
    // code
    return SpecificData.get();
  }

  public Conversion<?> getConversion(int field) {
    // for backward-compatibility. no older specific classes have conversions.
    return null;
  }

  @Override
  public void put(String fieldName, Object value) {
    Schema.Field field = getSchema().getField(fieldName);
    if (field == null) {
      throw new AIngleRuntimeException("Not a valid schema field: " + fieldName);
    }
    put(field.pos(), value);
  }

  @Override
  public Object get(String fieldName) {
    Schema.Field field = getSchema().getField(fieldName);
    if (field == null) {
      throw new AIngleRuntimeException("Not a valid schema field: " + fieldName);
    }
    return get(field.pos());
  }

  public Conversion<?> getConversion(String fieldName) {
    return getConversion(getSchema().getField(fieldName).pos());
  }

  @Override
  public boolean equals(Object that) {
    if (that == this)
      return true; // identical object
    if (!(that instanceof SpecificRecord))
      return false; // not a record
    if (this.getClass() != that.getClass())
      return false; // not same schema
    return getSpecificData().compare(this, that, this.getSchema(), true) == 0;
  }

  @Override
  public int hashCode() {
    return getSpecificData().hashCode(this, this.getSchema());
  }

  @Override
  public int compareTo(SpecificRecord that) {
    return getSpecificData().compare(this, that, this.getSchema());
  }

  @Override
  public String toString() {
    return getSpecificData().toString(this);
  }

  @Override
  public void writeExternal(ObjectOutput out) throws IOException {
    new SpecificDatumWriter(getSchema()).write(this, SpecificData.getEncoder(out));
  }

  @Override
  public void readExternal(ObjectInput in) throws IOException {
    new SpecificDatumReader(getSchema()).read(this, SpecificData.getDecoder(in));
  }

  /**
   * Returns true iff an instance supports the {@link MessageEncoder#encode} and
   * {@link MessageDecoder#decode} operations. Should only be used by
   * <code>SpecificDatumReader/Writer</code> to selectively use
   * {@link #customEncode} and {@link #customDecode} to optimize the
   * (de)serialization of values.
   */
  protected boolean hasCustomCoders() {
    return false;
  }

  public void customEncode(Encoder out) throws IOException {
    throw new UnsupportedOperationException();
  }

  public void customDecode(ResolvingDecoder in) throws IOException {
    throw new UnsupportedOperationException();
  }
}
