/*

 */
package org.apache.aingle.protobuf;

import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.Encoder;

import com.google.protobuf.ByteString;
import com.google.protobuf.Descriptors.EnumValueDescriptor;

/**
 * {@link org.apache.aingle.io.DatumWriter DatumWriter} for generated protobuf
 * classes.
 */
public class ProtobufDatumWriter<T> extends GenericDatumWriter<T> {
  public ProtobufDatumWriter() {
    super(ProtobufData.get());
  }

  public ProtobufDatumWriter(Class<T> c) {
    super(ProtobufData.get().getSchema(c), ProtobufData.get());
  }

  public ProtobufDatumWriter(Schema schema) {
    super(schema, ProtobufData.get());
  }

  protected ProtobufDatumWriter(Schema root, ProtobufData protobufData) {
    super(root, protobufData);
  }

  protected ProtobufDatumWriter(ProtobufData protobufData) {
    super(protobufData);
  }

  @Override
  protected void writeEnum(Schema schema, Object datum, Encoder out) throws IOException {
    if (!(datum instanceof EnumValueDescriptor))
      super.writeEnum(schema, datum, out); // punt to generic
    else
      out.writeEnum(schema.getEnumOrdinal(((EnumValueDescriptor) datum).getName()));
  }

  @Override
  protected void writeBytes(Object datum, Encoder out) throws IOException {
    ByteString bytes = (ByteString) datum;
    out.writeBytes(bytes.toByteArray(), 0, bytes.size());
  }

}
