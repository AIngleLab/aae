/*

 */
package org.apache.aingle;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.LineNumberReader;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Objects;
import java.util.Random;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.ipc.HttpTransceiver;
import org.apache.aingle.ipc.RPCContext;
import org.apache.aingle.ipc.RPCPlugin;
import org.apache.aingle.ipc.Requestor;
import org.apache.aingle.ipc.Responder;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.SocketServer;
import org.apache.aingle.ipc.SocketTransceiver;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.generic.GenericRequestor;
import org.apache.aingle.ipc.specific.SpecificRequestor;
import org.apache.aingle.ipc.specific.SpecificResponder;
import org.apache.aingle.specific.SpecificData;
import org.apache.aingle.test.Kind;
import org.apache.aingle.test.MD5;
import org.apache.aingle.test.Simple;
import org.apache.aingle.test.TestError;
import org.apache.aingle.test.TestRecord;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;

public class TestProtocolSpecific {

  protected static final int REPEATING = -1;

  public static int ackCount;

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

    public ByteBuffer echoBytes(ByteBuffer data) {
      return data;
    }

    public void error() throws TestError {
      if (throwUndeclaredError)
        throw new RuntimeException("foo");
      throw TestError.newBuilder().setMessage$("an error").build();
    }

    public void ack() {
      ackCount++;
    }
  }

  protected static Server server;
  protected static Transceiver client;
  protected static Simple proxy;

  protected static SpecificResponder responder;

  protected static HandshakeMonitor monitor;

  @Before
  public void testStartServer() throws Exception {
    if (server != null)
      return;
    responder = new SpecificResponder(Simple.class, new TestImpl());
    server = createServer(responder);
    server.start();

    client = createTransceiver();
    SpecificRequestor req = new SpecificRequestor(Simple.class, client);
    addRpcPlugins(req);
    proxy = SpecificRequestor.getClient(Simple.class, req);

    monitor = new HandshakeMonitor();
    responder.addRPCPlugin(monitor);
  }

  public void addRpcPlugins(Requestor requestor) {
  }

  public Server createServer(Responder testResponder) throws Exception {
    return server = new SocketServer(testResponder, new InetSocketAddress(0));
  }

  public Transceiver createTransceiver() throws Exception {
    return new SocketTransceiver(new InetSocketAddress(server.getPort()));
  }

  @Test
  public void testClassLoader() throws Exception {
    ClassLoader loader = new ClassLoader() {
    };

    SpecificResponder responder = new SpecificResponder(Simple.class, new TestImpl(), new SpecificData(loader));
    assertEquals(responder.getSpecificData().getClassLoader(), loader);

    SpecificRequestor requestor = new SpecificRequestor(Simple.class, client, new SpecificData(loader));
    assertEquals(requestor.getSpecificData().getClassLoader(), loader);
  }

  @Test
  public void testGetRemote() throws IOException {
    assertEquals(Simple.PROTOCOL, SpecificRequestor.getRemote(proxy));
  }

  @Test
  public void testHello() throws IOException {
    String response = proxy.hello("bob");
    assertEquals("goodbye", response);
  }

  @Test
  public void testHashCode() throws IOException {
    TestError error = new TestError();
    error.hashCode();
  }

  @Test
  public void testEcho() throws IOException {
    TestRecord record = new TestRecord();
    record.setName("foo");
    record.setKind(Kind.BAR);
    record.setHash(new MD5(new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 }));
    TestRecord echoed = proxy.echo(record);
    assertEquals(record, echoed);
    assertEquals(record.hashCode(), echoed.hashCode());
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
    ByteBuffer data = ByteBuffer.allocate(length);
    random.nextBytes(data.array());
    data.flip();
    ByteBuffer echoed = proxy.echoBytes(data);
    assertEquals(data, echoed);
  }

  @Test
  public void testEmptyEchoBytes() throws IOException {
    ByteBuffer data = ByteBuffer.allocate(0);
    ByteBuffer echoed = proxy.echoBytes(data);
    data.flip();
    assertEquals(data, echoed);
  }

  @Test
  public void testError() throws IOException {
    TestError error = null;
    try {
      proxy.error();
    } catch (TestError e) {
      error = e;
    }
    assertNotNull(error);
    assertEquals("an error", error.getMessage$());
  }

  @Test
  public void testUndeclaredError() throws Exception {
    this.throwUndeclaredError = true;
    RuntimeException error = null;
    try {
      proxy.error();
    } catch (RuntimeException e) {
      error = e;
    } finally {
      this.throwUndeclaredError = false;
    }
    assertNotNull(error);
    assertTrue(error.toString().contains("foo"));
  }

  @Test
  public void testOneWay() throws IOException {
    ackCount = 0;
    proxy.ack();
    proxy.hello("foo"); // intermix normal req
    proxy.ack();
    try {
      Thread.sleep(100);
    } catch (InterruptedException e) {
    }
    assertEquals(2, ackCount);
  }

  @Test
  public void testRepeatedAccess() throws Exception {
    for (int x = 0; x < 1000; x++) {
      proxy.hello("hi!");
    }
  }

  @Test(expected = Exception.class)
  public void testConnectionRefusedOneWay() throws IOException {
    Transceiver client = new HttpTransceiver(new URL("http://localhost:4444"));
    SpecificRequestor req = new SpecificRequestor(Simple.class, client);
    addRpcPlugins(req);
    Simple proxy = SpecificRequestor.getClient(Simple.class, req);
    proxy.ack();
  }

  @Test
  /**
   * Construct and use a protocol whose "hello" method has an extra argument to
   * check that schema is sent to parse request.
   */
  public void testParamVariation() throws Exception {
    Protocol protocol = new Protocol("Simple", "org.apache.aingle.test");
    List<Schema.Field> fields = new ArrayList<>();
    fields.add(new Schema.Field("extra", Schema.create(Schema.Type.BOOLEAN), null, null));
    fields.add(new Schema.Field("greeting", Schema.create(Schema.Type.STRING), null, null));
    Protocol.Message message = protocol.createMessage("hello", null /* doc */, new LinkedHashMap<String, String>(),
        Schema.createRecord(fields), Schema.create(Schema.Type.STRING), Schema.createUnion(new ArrayList<>()));
    protocol.getMessages().put("hello", message);
    try (Transceiver t = createTransceiver()) {
      GenericRequestor r = new GenericRequestor(protocol, t);
      addRpcPlugins(r);
      GenericRecord params = new GenericData.Record(message.getRequest());
      params.put("extra", Boolean.TRUE);
      params.put("greeting", "bob");
      String response = r.request("hello", params).toString();
      assertEquals("goodbye", response);
    }
  }

  @AfterClass
  public static void testHandshakeCount() throws IOException {
    monitor.assertHandshake();
  }

  @AfterClass
  public static void testStopServer() throws IOException {
    client.close();
    server.close();
    server = null;
  }

  public class HandshakeMonitor extends RPCPlugin {

    private int handshakes;
    private HashSet<String> seenProtocols = new HashSet<>();

    @Override
    public void serverConnecting(RPCContext context) {
      handshakes++;
      int expected = getExpectedHandshakeCount();
      if (expected > 0 && handshakes > expected) {
        throw new IllegalStateException(
            "Expected number of Protocol negotiation handshakes exceeded expected " + expected + " was " + handshakes);
      }

      // check that a given client protocol is only sent once
      String clientProtocol = context.getHandshakeRequest().getClientProtocol();
      if (clientProtocol != null) {
        assertFalse(seenProtocols.contains(clientProtocol));
        seenProtocols.add(clientProtocol);
      }
    }

    public void assertHandshake() {
      int expected = getExpectedHandshakeCount();
      if (expected != REPEATING) {
        assertEquals("Expected number of handshakes did not take place.", expected, handshakes);
      }
    }
  }

  protected int getExpectedHandshakeCount() {
    return 3;
  }

  public static class InteropTest {

    private static File SERVER_PORTS_DIR;
    static {
      try {
        SERVER_PORTS_DIR = Files.createTempDirectory(TestProtocolSpecific.class.getSimpleName()).toFile();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }

    @Test
    public void testClient() throws Exception {
      for (File f : Objects.requireNonNull(SERVER_PORTS_DIR.listFiles())) {
        try (LineNumberReader reader = new LineNumberReader(new FileReader(f))) {
          int port = Integer.parseInt(reader.readLine());
          System.out.println("Validating java client to " + f.getName() + " - " + port);
          Transceiver client = new SocketTransceiver(new InetSocketAddress("localhost", port));
          proxy = SpecificRequestor.getClient(Simple.class, client);
          TestProtocolSpecific proto = new TestProtocolSpecific();
          proto.testHello();
          proto.testEcho();
          proto.testEchoBytes();
          proto.testError();
          System.out.println("Done! Validation java client to " + f.getName() + " - " + port);
        }
      }
    }

    /**
     * Starts the RPC server.
     */
    public static void main(String[] args) throws Exception {
      SocketServer server = new SocketServer(new SpecificResponder(Simple.class, new TestImpl()),
          new InetSocketAddress(0));
      server.start();
      File portFile = new File(SERVER_PORTS_DIR, "java-port");
      try (FileWriter w = new FileWriter(portFile)) {
        w.write(Integer.toString(server.getPort()));
      }

    }
  }
}
