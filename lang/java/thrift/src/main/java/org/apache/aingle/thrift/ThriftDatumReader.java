/*

 */
package org.apache.aingle.thrift;

import java.io.IOException;
import java.util.Set;
import java.util.HashSet;

import org.apache.aingle.Schema;
import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.specific.SpecificData;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.util.ClassUtils;

/**
 * {@link org.apache.aingle.io.DatumReader DatumReader} for generated Thrift
 * classes.
 */
public class ThriftDatumReader<T> extends GenericDatumReader<T> {
  public ThriftDatumReader() {
    this(null, null, ThriftData.get());
  }

  public ThriftDatumReader(Class<T> c) {
    this(ThriftData.get().getSchema(c));
  }

  /** Construct where the writer's and reader's schemas are the same. */
  public ThriftDatumReader(Schema schema) {
    this(schema, schema, ThriftData.get());
  }

  /** Construct given writer's and reader's schema. */
  public ThriftDatumReader(Schema writer, Schema reader) {
    this(writer, reader, ThriftData.get());
  }

  protected ThriftDatumReader(Schema writer, Schema reader, ThriftData data) {
    super(writer, reader, data);
  }

  @Override
  protected Object createEnum(String symbol, Schema schema) {
    try {
      Class c = ClassUtils.forName(SpecificData.getClassName(schema));
      if (c == null)
        return super.createEnum(symbol, schema); // punt to generic
      return Enum.valueOf(c, symbol);
    } catch (Exception e) {
      throw new AIngleRuntimeException(e);
    }
  }

  @Override
  protected Object readInt(Object old, Schema s, Decoder in) throws IOException {
    String type = s.getProp(ThriftData.THRIFT_PROP);
    int value = in.readInt();
    if (type != null) {
      if ("byte".equals(type))
        return (byte) value;
      if ("short".equals(type))
        return (short) value;
    }
    return value;
  }

  @Override
  protected Object newArray(Object old, int size, Schema schema) {
    if ("set".equals(schema.getProp(ThriftData.THRIFT_PROP))) {
      if (old instanceof Set) {
        ((Set) old).clear();
        return old;
      }
      return new HashSet();
    } else {
      return super.newArray(old, size, schema);
    }
  }

}
