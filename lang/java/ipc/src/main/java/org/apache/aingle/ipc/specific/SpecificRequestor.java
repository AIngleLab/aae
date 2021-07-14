/*

 */

package org.apache.aingle.ipc.specific;

import java.io.IOException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Type;
import java.util.Arrays;

import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.Requestor;
import org.apache.aingle.ipc.Callback;
import org.apache.aingle.specific.SpecificData;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.specific.SpecificDatumWriter;

/** {@link org.apache.aingle.ipc.Requestor Requestor} for generated interfaces. */
public class SpecificRequestor extends Requestor implements InvocationHandler {
  SpecificData data;

  public SpecificRequestor(Class<?> iface, Transceiver transceiver) throws IOException {
    this(iface, transceiver, new SpecificData(iface.getClassLoader()));
  }

  protected SpecificRequestor(Protocol protocol, Transceiver transceiver) throws IOException {
    this(protocol, transceiver, SpecificData.get());
  }

  public SpecificRequestor(Class<?> iface, Transceiver transceiver, SpecificData data) throws IOException {
    this(data.getProtocol(iface), transceiver, data);
  }

  public SpecificRequestor(Protocol protocol, Transceiver transceiver, SpecificData data) throws IOException {
    super(protocol, transceiver);
    this.data = data;
  }

  public SpecificData getSpecificData() {
    return data;
  }

  @Override
  public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
    String name = method.getName();
    switch (name) {
    case "hashCode":
      return hashCode();
    case "equals":
      Object obj = args[0];
      return (proxy == obj)
          || (obj != null && Proxy.isProxyClass(obj.getClass()) && this.equals(Proxy.getInvocationHandler(obj)));
    case "toString":
      String protocol = "unknown";
      String remote = "unknown";
      Class<?>[] interfaces = proxy.getClass().getInterfaces();
      if (interfaces.length > 0) {
        try {
          protocol = Class.forName(interfaces[0].getName()).getSimpleName();
        } catch (ClassNotFoundException e) {
        }

        InvocationHandler handler = Proxy.getInvocationHandler(proxy);
        if (handler instanceof Requestor) {
          try {
            remote = ((Requestor) handler).getTransceiver().getRemoteName();
          } catch (IOException e) {
          }
        }
      }
      return "Proxy[" + protocol + "," + remote + "]";
    default:
      try {
        // Check if this is a callback-based RPC:
        Type[] parameterTypes = method.getParameterTypes();
        if ((parameterTypes.length > 0) && (parameterTypes[parameterTypes.length - 1] instanceof Class)
            && Callback.class.isAssignableFrom(((Class<?>) parameterTypes[parameterTypes.length - 1]))) {
          // Extract the Callback from the end of of the argument list
          Object[] finalArgs = Arrays.copyOf(args, args.length - 1);
          Callback<?> callback = (Callback<?>) args[args.length - 1];
          request(method.getName(), finalArgs, callback);
          return null;
        } else {
          return request(method.getName(), args);
        }
      } catch (Exception e) {
        // Check if this is a declared Exception:
        for (Class<?> exceptionClass : method.getExceptionTypes()) {
          if (exceptionClass.isAssignableFrom(e.getClass())) {
            throw e;
          }
        }

        // Next, check for RuntimeExceptions:
        if (e instanceof RuntimeException) {
          throw e;
        }

        // Not an expected Exception, so wrap it in AIngleRuntimeException:
        throw new AIngleRuntimeException(e);
      }
    }
  }

  protected DatumWriter<Object> getDatumWriter(Schema schema) {
    return new SpecificDatumWriter<>(schema, data);
  }

  @Deprecated // for compatibility in 1.5
  protected DatumReader<Object> getDatumReader(Schema schema) {
    return getDatumReader(schema, schema);
  }

  protected DatumReader<Object> getDatumReader(Schema writer, Schema reader) {
    return new SpecificDatumReader<>(writer, reader, data);
  }

  @Override
  public void writeRequest(Schema schema, Object request, Encoder out) throws IOException {
    Object[] args = (Object[]) request;
    int i = 0;
    for (Schema.Field param : schema.getFields())
      getDatumWriter(param.schema()).write(args[i++], out);
  }

  @Override
  public Object readResponse(Schema writer, Schema reader, Decoder in) throws IOException {
    return getDatumReader(writer, reader).read(null, in);
  }

  @Override
  public Exception readError(Schema writer, Schema reader, Decoder in) throws IOException {
    Object value = getDatumReader(writer, reader).read(null, in);
    if (value instanceof Exception)
      return (Exception) value;
    return new AIngleRuntimeException(value.toString());
  }

  /** Create a proxy instance whose methods invoke RPCs. */
  public static <T> T getClient(Class<T> iface, Transceiver transceiver) throws IOException {
    return getClient(iface, transceiver, new SpecificData(iface.getClassLoader()));
  }

  /** Create a proxy instance whose methods invoke RPCs. */
  @SuppressWarnings("unchecked")
  public static <T> T getClient(Class<T> iface, Transceiver transceiver, SpecificData data) throws IOException {
    Protocol protocol = data.getProtocol(iface);
    return (T) Proxy.newProxyInstance(data.getClassLoader(), new Class[] { iface },
        new SpecificRequestor(protocol, transceiver, data));
  }

  /** Create a proxy instance whose methods invoke RPCs. */
  @SuppressWarnings("unchecked")
  public static <T> T getClient(Class<T> iface, SpecificRequestor requestor) throws IOException {
    return (T) Proxy.newProxyInstance(requestor.data.getClassLoader(), new Class[] { iface }, requestor);
  }

  /** Return the remote protocol for a proxy. */
  public static Protocol getRemote(Object proxy) throws IOException {
    return ((Requestor) Proxy.getInvocationHandler(proxy)).getRemote();

  }

}
