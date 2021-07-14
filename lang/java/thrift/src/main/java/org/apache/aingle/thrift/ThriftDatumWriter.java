/*

 */
package org.apache.aingle.thrift;

import org.apache.aingle.Schema;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.generic.GenericDatumWriter;

import java.nio.ByteBuffer;
import java.io.IOException;

/**
 * {@link org.apache.aingle.io.DatumWriter DatumWriter} for generated thrift
 * classes.
 */
public class ThriftDatumWriter<T> extends GenericDatumWriter<T> {
  public ThriftDatumWriter() {
    super(ThriftData.get());
  }

  public ThriftDatumWriter(Class<T> c) {
    super(ThriftData.get().getSchema(c), ThriftData.get());
  }

  public ThriftDatumWriter(Schema schema) {
    super(schema, ThriftData.get());
  }

  protected ThriftDatumWriter(Schema root, ThriftData thriftData) {
    super(root, thriftData);
  }

  protected ThriftDatumWriter(ThriftData thriftData) {
    super(thriftData);
  }

  @Override
  protected void writeBytes(Object datum, Encoder out) throws IOException {
    // Thrift assymetry: setter takes ByteBuffer but getter returns byte[]
    out.writeBytes(ByteBuffer.wrap((byte[]) datum));
  }

}
