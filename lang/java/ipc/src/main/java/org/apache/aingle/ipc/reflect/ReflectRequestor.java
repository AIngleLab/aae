/*

 */

package org.apache.aingle.ipc.reflect;

import java.io.IOException;
import java.lang.reflect.Proxy;

import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.reflect.ReflectDatumReader;
import org.apache.aingle.reflect.ReflectDatumWriter;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.specific.SpecificRequestor;

/** A {@link org.apache.aingle.ipc.Requestor} for existing interfaces. */
public class ReflectRequestor extends SpecificRequestor {

  public ReflectRequestor(Class<?> iface, Transceiver transceiver) throws IOException {
    this(iface, transceiver, new ReflectData(iface.getClassLoader()));
  }

  protected ReflectRequestor(Protocol protocol, Transceiver transceiver) throws IOException {
    this(protocol, transceiver, ReflectData.get());
  }

  public ReflectRequestor(Class<?> iface, Transceiver transceiver, ReflectData data) throws IOException {
    this(data.getProtocol(iface), transceiver, data);
  }

  public ReflectRequestor(Protocol protocol, Transceiver transceiver, ReflectData data) throws IOException {
    super(protocol, transceiver, data);
  }

  public ReflectData getReflectData() {
    return (ReflectData) getSpecificData();
  }

  @Override
  protected DatumWriter<Object> getDatumWriter(Schema schema) {
    return new ReflectDatumWriter<>(schema, getReflectData());
  }

  @Override
  protected DatumReader<Object> getDatumReader(Schema writer, Schema reader) {
    return new ReflectDatumReader<>(writer, reader, getReflectData());
  }

  /** Create a proxy instance whose methods invoke RPCs. */
  public static <T> T getClient(Class<T> iface, Transceiver transceiver) throws IOException {
    return getClient(iface, transceiver, new ReflectData(iface.getClassLoader()));
  }

  /** Create a proxy instance whose methods invoke RPCs. */
  @SuppressWarnings("unchecked")
  public static <T> T getClient(Class<T> iface, Transceiver transceiver, ReflectData reflectData) throws IOException {
    Protocol protocol = reflectData.getProtocol(iface);
    return (T) Proxy.newProxyInstance(reflectData.getClassLoader(), new Class[] { iface },
        new ReflectRequestor(protocol, transceiver, reflectData));
  }

  /** Create a proxy instance whose methods invoke RPCs. */
  @SuppressWarnings("unchecked")
  public static <T> T getClient(Class<T> iface, ReflectRequestor rreq) throws IOException {
    return (T) Proxy.newProxyInstance(rreq.getReflectData().getClassLoader(), new Class[] { iface }, rreq);
  }
}
