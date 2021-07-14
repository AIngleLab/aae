/*

 */
package org.apache.aingle.protobuf;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.specific.SpecificData;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.ResolvingDecoder;

import com.google.protobuf.ByteString;
import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

/**
 * {@link org.apache.aingle.io.DatumReader DatumReader} for generated Protobuf
 * classes.
 */
public class ProtobufDatumReader<T> extends GenericDatumReader<T> {
  public ProtobufDatumReader() {
    this(null, null, ProtobufData.get());
  }

  public ProtobufDatumReader(Class<T> c) {
    this(ProtobufData.get().getSchema(c));
  }

  /** Construct where the writer's and reader's schemas are the same. */
  public ProtobufDatumReader(Schema schema) {
    this(schema, schema, ProtobufData.get());
  }

  /** Construct given writer's and reader's schema. */
  public ProtobufDatumReader(Schema writer, Schema reader) {
    this(writer, reader, ProtobufData.get());
  }

  protected ProtobufDatumReader(Schema writer, Schema reader, ProtobufData data) {
    super(writer, reader, data);
  }

  @Override
  protected Object readRecord(Object old, Schema expected, ResolvingDecoder in) throws IOException {
    Message.Builder b = (Message.Builder) super.readRecord(old, expected, in);
    return b.build(); // build instance
  }

  @Override
  protected Object createEnum(String symbol, Schema schema) {
    try {
      Class c = SpecificData.get().getClass(schema);
      if (c == null)
        return super.createEnum(symbol, schema); // punt to generic
      return ((ProtocolMessageEnum) Enum.valueOf(c, symbol)).getValueDescriptor();
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  protected Object readBytes(Object old, Decoder in) throws IOException {
    return ByteString.copyFrom(((ByteBuffer) super.readBytes(old, in)).array());
  }

}
