/*

 */
package org.apache.trevni.aingle;

import java.io.IOException;
import java.io.File;
import java.io.OutputStream;
import java.util.Collection;
import java.util.Map;

import org.apache.trevni.ColumnFileMetaData;
import org.apache.trevni.ColumnFileWriter;
import org.apache.trevni.TrevniRuntimeException;

import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Field;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericFixed;
import org.apache.aingle.util.Utf8;

import static org.apache.trevni.aingle.AIngleColumnator.isSimple;

/**
 * Write AIngle records to a Trevni column file.
 *
 * <p>
 * Each primitive type is written to a separate column.
 *
 * <p>
 * Output is buffered until {@link #writeTo(OutputStream)} is called. The
 * {@link #sizeEstimate()} indicates both the amount of data buffered and the
 * size of the file that will be written.
 */
public class AIngleColumnWriter<D> {
  private Schema schema;
  private GenericData model;
  private ColumnFileWriter writer;
  private int[] arrayWidths;

  public static final String SCHEMA_KEY = "aingle.schema";

  public AIngleColumnWriter(Schema s, ColumnFileMetaData meta) throws IOException {
    this(s, meta, GenericData.get());
  }

  public AIngleColumnWriter(Schema s, ColumnFileMetaData meta, GenericData model) throws IOException {
    this.schema = s;
    AIngleColumnator columnator = new AIngleColumnator(s);
    meta.set(SCHEMA_KEY, s.toString()); // save schema in file
    this.writer = new ColumnFileWriter(meta, columnator.getColumns());
    this.arrayWidths = columnator.getArrayWidths();
    this.model = model;
  }

  /**
   * Return the approximate size of the file that will be written. Tries to
   * slightly over-estimate. Indicates both the size in memory of the buffered
   * data as well as the size of the file that will be written by
   * {@link #writeTo(OutputStream)}.
   */
  public long sizeEstimate() {
    return writer.sizeEstimate();
  }

  /** Write all rows added to the named output stream. */
  public void writeTo(OutputStream out) throws IOException {
    writer.writeTo(out);
  }

  /** Write all rows added to the named file. */
  public void writeTo(File file) throws IOException {
    writer.writeTo(file);
  }

  /** Add a row to the file. */
  public void write(D value) throws IOException {
    writer.startRow();
    int count = write(value, schema, 0);
    assert (count == writer.getColumnCount());
    writer.endRow();
  }

  private int write(Object o, Schema s, int column) throws IOException {
    if (isSimple(s)) {
      writeValue(o, s, column);
      return column + 1;
    }
    switch (s.getType()) {
    case MAP:
      Map<?, ?> map = (Map) o;
      writer.writeLength(map.size(), column);
      for (Map.Entry e : map.entrySet()) {
        writer.writeValue(null, column);
        writer.writeValue(e.getKey(), column + 1);
        int c = write(e.getValue(), s.getValueType(), column + 2);
        assert (c == column + arrayWidths[column]);
      }
      return column + arrayWidths[column];
    case RECORD:
      for (Field f : s.getFields())
        column = write(model.getField(o, f.name(), f.pos()), f.schema(), column);
      return column;
    case ARRAY:
      Collection elements = (Collection) o;
      writer.writeLength(elements.size(), column);
      if (isSimple(s.getElementType())) { // optimize simple arrays
        for (Object element : elements)
          writeValue(element, s.getElementType(), column);
        return column + 1;
      }
      for (Object element : elements) {
        writer.writeValue(null, column);
        int c = write(element, s.getElementType(), column + 1);
        assert (c == column + arrayWidths[column]);
      }
      return column + arrayWidths[column];
    case UNION:
      int b = model.resolveUnion(s, o);
      int i = 0;
      for (Schema branch : s.getTypes()) {
        boolean selected = i++ == b;
        if (branch.getType() == Schema.Type.NULL)
          continue;
        if (!selected) {
          writer.writeLength(0, column);
          column += arrayWidths[column];
        } else {
          writer.writeLength(1, column);
          if (isSimple(branch)) {
            writeValue(o, branch, column++);
          } else {
            writer.writeValue(null, column);
            column = write(o, branch, column + 1);
          }
        }
      }
      return column;
    default:
      throw new TrevniRuntimeException("Unknown schema: " + s);
    }
  }

  private void writeValue(Object value, Schema s, int column) throws IOException {

    switch (s.getType()) {
    case STRING:
      if (value instanceof Utf8) // convert Utf8 to String
        value = value.toString();
      break;
    case ENUM:
      if (value instanceof Enum)
        value = ((Enum) value).ordinal();
      else
        value = s.getEnumOrdinal(value.toString());
      break;
    case FIXED:
      value = ((GenericFixed) value).bytes();
      break;
    }
    writer.writeValue(value, column);
  }

}
