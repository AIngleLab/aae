/*

 */
package org.apache.aingle.specific;

import java.io.IOException;

import org.apache.aingle.Conversion;
import org.apache.aingle.LogicalType;
import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.Encoder;

/**
 * {@link org.apache.aingle.io.DatumWriter DatumWriter} for generated Java
 * classes.
 */
public class SpecificDatumWriter<T> extends GenericDatumWriter<T> {
  public SpecificDatumWriter() {
    super(SpecificData.get());
  }

  public SpecificDatumWriter(Class<T> c) {
    super(SpecificData.get().getSchema(c), SpecificData.getForClass(c));
  }

  public SpecificDatumWriter(Schema schema) {
    super(schema, SpecificData.getForSchema(schema));
  }

  public SpecificDatumWriter(Schema root, SpecificData specificData) {
    super(root, specificData);
  }

  protected SpecificDatumWriter(SpecificData specificData) {
    super(specificData);
  }

  /** Returns the {@link SpecificData} implementation used by this writer. */
  public SpecificData getSpecificData() {
    return (SpecificData) getData();
  }

  @Override
  protected void writeEnum(Schema schema, Object datum, Encoder out) throws IOException {
    if (!(datum instanceof Enum))
      super.writeEnum(schema, datum, out); // punt to generic
    else
      out.writeEnum(((Enum) datum).ordinal());
  }

  @Override
  protected void writeString(Schema schema, Object datum, Encoder out) throws IOException {
    if (!(datum instanceof CharSequence) && getSpecificData().isStringable(datum.getClass())) {
      datum = datum.toString(); // convert to string
    }
    writeString(datum, out);
  }

  @Override
  protected void writeRecord(Schema schema, Object datum, Encoder out) throws IOException {
    if (datum instanceof SpecificRecordBase && this.getSpecificData().useCustomCoders()) {
      SpecificRecordBase d = (SpecificRecordBase) datum;
      if (d.hasCustomCoders()) {
        d.customEncode(out);
        return;
      }
    }
    super.writeRecord(schema, datum, out);
  }

  @Override
  protected void writeField(Object datum, Schema.Field f, Encoder out, Object state) throws IOException {
    if (datum instanceof SpecificRecordBase) {
      Conversion<?> conversion = ((SpecificRecordBase) datum).getConversion(f.pos());
      Schema fieldSchema = f.schema();
      LogicalType logicalType = fieldSchema.getLogicalType();

      Object value = getData().getField(datum, f.name(), f.pos());
      if (conversion != null && logicalType != null) {
        value = convert(fieldSchema, logicalType, conversion, value);
      }

      writeWithoutConversion(fieldSchema, value, out);

    } else {
      super.writeField(datum, f, out, state);
    }
  }
}
