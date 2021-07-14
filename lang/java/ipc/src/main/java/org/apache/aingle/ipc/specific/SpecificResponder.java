/*

 */

package org.apache.aingle.ipc.specific;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import org.apache.aingle.Schema;
import org.apache.aingle.Protocol;
import org.apache.aingle.Protocol.Message;
import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.specific.SpecificData;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.specific.SpecificDatumWriter;
import org.apache.aingle.ipc.generic.GenericResponder;

/** {@link org.apache.aingle.ipc.Responder Responder} for generated interfaces. */
public class SpecificResponder extends GenericResponder {
  private Object impl;

  public SpecificResponder(Class iface, Object impl) {
    this(iface, impl, new SpecificData(impl.getClass().getClassLoader()));
  }

  public SpecificResponder(Protocol protocol, Object impl) {
    this(protocol, impl, new SpecificData(impl.getClass().getClassLoader()));
  }

  public SpecificResponder(Class iface, Object impl, SpecificData data) {
    this(data.getProtocol(iface), impl, data);
  }

  public SpecificResponder(Protocol protocol, Object impl, SpecificData data) {
    super(protocol, data);
    this.impl = impl;
  }

  public SpecificData getSpecificData() {
    return (SpecificData) getGenericData();
  }

  @Override
  protected DatumWriter<Object> getDatumWriter(Schema schema) {
    return new SpecificDatumWriter<>(schema, getSpecificData());
  }

  @Override
  protected DatumReader<Object> getDatumReader(Schema actual, Schema expected) {
    return new SpecificDatumReader<>(actual, expected, getSpecificData());
  }

  @Override
  public void writeError(Schema schema, Object error, Encoder out) throws IOException {
    getDatumWriter(schema).write(error, out);
  }

  @Override
  public Object respond(Message message, Object request) throws Exception {
    int numParams = message.getRequest().getFields().size();
    Object[] params = new Object[numParams];
    Class[] paramTypes = new Class[numParams];
    int i = 0;
    try {
      for (Schema.Field param : message.getRequest().getFields()) {
        params[i] = ((GenericRecord) request).get(param.name());
        paramTypes[i] = getSpecificData().getClass(param.schema());
        i++;
      }
      Method method = impl.getClass().getMethod(message.getName(), paramTypes);
      method.setAccessible(true);
      return method.invoke(impl, params);
    } catch (InvocationTargetException e) {
      Throwable error = e.getTargetException();
      if (error instanceof Exception) {
        throw (Exception) error;
      } else {
        throw new AIngleRuntimeException(error);
      }
    } catch (NoSuchMethodException | IllegalAccessException e) {
      throw new AIngleRuntimeException(e);
    }
  }

}
