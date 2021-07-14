/*

 */
package org.apache.aingle;

import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.SocketServer;
import org.apache.aingle.ipc.SocketTransceiver;
import org.apache.aingle.ipc.reflect.ReflectRequestor;
import org.apache.aingle.ipc.reflect.ReflectResponder;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.net.InetSocketAddress;
import java.util.Random;
import java.io.IOException;

public class TestProtocolReflect {

  public static class TestRecord {
    private String name;

    public int hashCode() {
      return this.name.hashCode();
    }

    public boolean equals(Object that) {
      return this.name.equals(((TestRecord) that).name);
    }
  }

  public interface Simple {
    String hello(String greeting);

    TestRecord echo(TestRecord record);

    int add(int arg1, int arg2);

    byte[] echoBytes(byte[] data);

    void error() throws SimpleException;
  }

  private static boolean throwUndeclaredError;

  public static class TestImpl implements Simple {
    public String hello(String greeting) {
      return "goodbye";
    }

    public int add(int arg1, int arg2) {
      return arg1 + arg2;
    }

    public TestRecord echo(TestRecord record) {
      return record;
    }

    public byte[] echoBytes(byte[] data) {
      return data;
    }

    public void error() throws SimpleException {
      if (throwUndeclaredError)
        throw new RuntimeException("foo");
      throw new SimpleException("foo");
    }
  }

  protected static Server server;
  protected static Transceiver client;
  protected static Simple proxy;

  @Before
  public void testStartServer() throws Exception {
    if (server != null)
      return;
    server = new SocketServer(new ReflectResponder(Simple.class, new TestImpl()), new InetSocketAddress(0));
    server.start();
    client = new SocketTransceiver(new InetSocketAddress(server.getPort()));
    proxy = ReflectRequestor.getClient(Simple.class, client);
  }

  @Test
  public void testClassLoader() throws Exception {
    ClassLoader loader = new ClassLoader() {
    };

    ReflectResponder responder = new ReflectResponder(Simple.class, new TestImpl(), new ReflectData(loader));
    assertEquals(responder.getReflectData().getClassLoader(), loader);

    ReflectRequestor requestor = new ReflectRequestor(Simple.class, client, new ReflectData(loader));
    assertEquals(requestor.getReflectData().getClassLoader(), loader);
  }

  @Test
  public void testHello() throws IOException {
    String response = proxy.hello("bob");
    assertEquals("goodbye", response);
  }

  @Test
  public void testEcho() throws IOException {
    TestRecord record = new TestRecord();
    record.name = "foo";
    TestRecord echoed = proxy.echo(record);
    assertEquals(record, echoed);
  }

  @Test
  public void testAdd() throws IOException {
    int result = proxy.add(1, 2);
    assertEquals(3, result);
  }

  @Test
  public void testEchoBytes() throws IOException {
    Random random = new Random();
    int length = random.nextInt(1024 * 16);
    byte[] data = new byte[length];
    random.nextBytes(data);
    byte[] echoed = proxy.echoBytes(data);
    assertArrayEquals(data, echoed);
  }

  @Test
  public void testError() throws IOException {
    SimpleException error = null;
    try {
      proxy.error();
    } catch (SimpleException e) {
      error = e;
    }
    assertNotNull(error);
    assertEquals("foo", error.getMessage());
  }

  @Test
  public void testUndeclaredError() throws Exception {
    this.throwUndeclaredError = true;
    RuntimeException error = null;
    try {
      proxy.error();
    } catch (AIngleRuntimeException e) {
      error = e;
    } finally {
      this.throwUndeclaredError = false;
    }
    assertNotNull(error);
    assertTrue(error.toString().contains("foo"));
  }

  @AfterClass
  public static void testStopServer() throws IOException {
    client.close();
    server.close();
  }

}
