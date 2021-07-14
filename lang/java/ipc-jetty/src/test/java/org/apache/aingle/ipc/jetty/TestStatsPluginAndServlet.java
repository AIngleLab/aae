/*

 */
package org.apache.aingle.ipc.jetty;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.io.StringWriter;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.Random;

import javax.servlet.UnavailableException;

import org.apache.aingle.AIngleRemoteException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Protocol.Message;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.ipc.HttpTransceiver;
import org.apache.aingle.ipc.LocalTransceiver;
import org.apache.aingle.ipc.RPCContext;
import org.apache.aingle.ipc.Responder;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.generic.GenericRequestor;
import org.apache.aingle.ipc.generic.GenericResponder;
import org.apache.aingle.ipc.stats.StatsPlugin;
import org.apache.aingle.ipc.stats.StatsServlet;
import org.junit.Test;

public class TestStatsPluginAndServlet {
  Protocol protocol = Protocol.parse("" + "{\"protocol\": \"Minimal\", " + "\"messages\": { \"m\": {"
      + "   \"request\": [{\"name\": \"x\", \"type\": \"int\"}], " + "   \"response\": \"int\"} } }");
  Message message = protocol.getMessages().get("m");

  private static final long MS = 1000 * 1000L;

  /** Returns an HTML string. */
  private String generateServletResponse(StatsPlugin statsPlugin) throws IOException {
    StatsServlet servlet;
    try {
      servlet = new StatsServlet(statsPlugin);
    } catch (UnavailableException e1) {
      throw new IOException();
    }
    StringWriter w = new StringWriter();
    try {
      servlet.writeStats(w);
    } catch (Exception e) {
      e.printStackTrace();
    }
    String o = w.toString();
    return o;
  }

  /** Expects 0 and returns 1. */
  static class TestResponder extends GenericResponder {
    public TestResponder(Protocol local) {
      super(local);
    }

    @Override
    public Object respond(Message message, Object request) throws AIngleRemoteException {
      assertEquals(0, ((GenericRecord) request).get("x"));
      return 1;
    }

  }

  private void makeRequest(Transceiver t) throws Exception {
    GenericRecord params = new GenericData.Record(protocol.getMessages().get("m").getRequest());
    params.put("x", 0);
    GenericRequestor r = new GenericRequestor(protocol, t);
    assertEquals(1, r.request("m", params));
  }

  @Test
  public void testFullServerPath() throws Exception {
    Responder r = new TestResponder(protocol);
    StatsPlugin statsPlugin = new StatsPlugin();
    r.addRPCPlugin(statsPlugin);
    Transceiver t = new LocalTransceiver(r);

    for (int i = 0; i < 10; ++i) {
      makeRequest(t);
    }

    String o = generateServletResponse(statsPlugin);
    assertTrue(o.contains("10 calls"));
  }

  @Test
  public void testMultipleRPCs() throws IOException {
    org.apache.aingle.ipc.stats.FakeTicks t = new org.apache.aingle.ipc.stats.FakeTicks();
    StatsPlugin statsPlugin = new StatsPlugin(t, StatsPlugin.LATENCY_SEGMENTER, StatsPlugin.PAYLOAD_SEGMENTER);
    RPCContext context1 = makeContext();
    RPCContext context2 = makeContext();
    statsPlugin.serverReceiveRequest(context1);
    t.passTime(100 * MS); // first takes 100ms
    statsPlugin.serverReceiveRequest(context2);
    String r = generateServletResponse(statsPlugin);
    // Check in progress RPCs
    assertTrue(r.contains("m: 0ms"));
    assertTrue(r.contains("m: 100ms"));
    statsPlugin.serverSendResponse(context1);
    t.passTime(900 * MS); // second takes 900ms
    statsPlugin.serverSendResponse(context2);
    r = generateServletResponse(statsPlugin);
    assertTrue(r.contains("Average: 500.0ms"));
  }

  @Test
  public void testPayloadSize() throws Exception {
    Responder r = new TestResponder(protocol);
    StatsPlugin statsPlugin = new StatsPlugin();
    r.addRPCPlugin(statsPlugin);
    Transceiver t = new LocalTransceiver(r);
    makeRequest(t);

    String resp = generateServletResponse(statsPlugin);
    assertTrue(resp.contains("Average: 2.0"));

  }

  private RPCContext makeContext() {
    RPCContext context = new RPCContext();
    context.setMessage(message);
    return context;
  }

  /** Sleeps as requested. */
  private static class SleepyResponder extends GenericResponder {
    public SleepyResponder(Protocol local) {
      super(local);
    }

    @Override
    public Object respond(Message message, Object request) throws AIngleRemoteException {
      try {
        Thread.sleep((Long) ((GenericRecord) request).get("millis"));
      } catch (InterruptedException e) {
        throw new AIngleRemoteException(e);
      }
      return null;
    }
  }

  /**
   * Demo program for using RPC stats. This automatically generates client RPC
   * requests. Alternatively a can be used (as below) to trigger RPCs.
   * 
   * <pre>
   * java -jar build/aingle-tools-*.jar rpcsend '{"protocol":"sleepy","namespace":null,"types":[],"messages":{"sleep":{"request":[{"name":"millis","type":"long"}],"response":"null"}}}' sleep localhost 7002 '{"millis": 20000}'
   * </pre>
   * 
   * @param args
   * @throws Exception
   */
  public static void main(String[] args) throws Exception {
    if (args.length == 0) {
      args = new String[] { "7002", "7003" };
    }
    Protocol protocol = Protocol.parse("{\"protocol\": \"sleepy\", " + "\"messages\": { \"sleep\": {"
        + "   \"request\": [{\"name\": \"millis\", \"type\": \"long\"},"
        + "{\"name\": \"data\", \"type\": \"bytes\"}], " + "   \"response\": \"null\"} } }");
    Responder r = new SleepyResponder(protocol);
    StatsPlugin p = new StatsPlugin();
    r.addRPCPlugin(p);

    // Start AIngle server
    HttpServer aingleServer = new HttpServer(r, Integer.parseInt(args[0]));
    aingleServer.start();

    StatsServer ss = new StatsServer(p, 8080);

    HttpTransceiver trans = new HttpTransceiver(new URL("http://localhost:" + Integer.parseInt(args[0])));
    GenericRequestor req = new GenericRequestor(protocol, trans);

    while (true) {
      Thread.sleep(1000);
      GenericRecord params = new GenericData.Record(protocol.getMessages().get("sleep").getRequest());
      Random rand = new Random();
      params.put("millis", Math.abs(rand.nextLong()) % 1000);
      int payloadSize = Math.abs(rand.nextInt()) % 10000;
      byte[] payload = new byte[payloadSize];
      rand.nextBytes(payload);
      params.put("data", ByteBuffer.wrap(payload));
      req.request("sleep", params);
    }
  }
}
