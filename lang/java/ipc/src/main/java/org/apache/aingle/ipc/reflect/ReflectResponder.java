/*

 */

package org.apache.aingle.ipc.reflect;

import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.Protocol;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.ipc.specific.SpecificResponder;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.reflect.ReflectDatumReader;
import org.apache.aingle.reflect.ReflectDatumWriter;

/** {@link org.apache.aingle.ipc.Responder} for existing interfaces. */
public class ReflectResponder extends SpecificResponder {
  public ReflectResponder(Class iface, Object impl) {
    this(iface, impl, new ReflectData(impl.getClass().getClassLoader()));
  }

  public ReflectResponder(Protocol protocol, Object impl) {
    this(protocol, impl, new ReflectData(impl.getClass().getClassLoader()));
  }

  public ReflectResponder(Class iface, Object impl, ReflectData data) {
    this(data.getProtocol(iface), impl, data);
  }

  public ReflectResponder(Protocol protocol, Object impl, ReflectData data) {
    super(protocol, impl, data);
  }

  public ReflectData getReflectData() {
    return (ReflectData) getSpecificData();
  }

  @Override
  protected DatumWriter<Object> getDatumWriter(Schema schema) {
    return new ReflectDatumWriter<>(schema, getReflectData());
  }

  @Override
  protected DatumReader<Object> getDatumReader(Schema actual, Schema expected) {
    return new ReflectDatumReader<>(actual, expected, getReflectData());
  }

  @Override
  public void writeError(Schema schema, Object error, Encoder out) throws IOException {
    if (error instanceof CharSequence)
      error = error.toString(); // system error: convert
    super.writeError(schema, error, out);
  }

}
