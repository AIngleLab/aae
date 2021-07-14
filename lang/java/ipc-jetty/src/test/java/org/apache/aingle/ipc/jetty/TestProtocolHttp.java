/*

 */
package org.apache.aingle.ipc.jetty;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.TestProtocolSpecific;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.Responder;
import org.apache.aingle.ipc.HttpTransceiver;
import org.apache.aingle.ipc.generic.GenericRequestor;
import org.apache.aingle.ipc.specific.SpecificRequestor;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.test.Simple;

import org.junit.Test;

import java.net.URL;
import java.net.ServerSocket;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.LinkedHashMap;

public class TestProtocolHttp extends TestProtocolSpecific {

  @Override
  public Server createServer(Responder testResponder) throws Exception {
    return new HttpServer(testResponder, 0);
  }

  @Override
  public Transceiver createTransceiver() throws Exception {
    return new HttpTransceiver(new URL("http://127.0.0.1:" + server.getPort() + "/"));
  }

  protected int getExpectedHandshakeCount() {
    return REPEATING;
  }

  @Test
  public void testTimeout() throws Throwable {
    ServerSocket s = new ServerSocket(0);
    HttpTransceiver client = new HttpTransceiver(new URL("http://127.0.0.1:" + s.getLocalPort() + "/"));
    client.setTimeout(100);
    Simple proxy = SpecificRequestor.getClient(Simple.class, client);
    try {
      proxy.hello("foo");
      fail("Should have failed with an exception");
    } catch (AIngleRuntimeException e) {
      assertTrue("Got unwanted exception: " + e.getCause(), e.getCause() instanceof SocketTimeoutException);
    } finally {
      s.close();
    }
  }

  /** Test that Responder ignores one-way with stateless transport. */
  @Test
  public void testStatelessOneway() throws Exception {
    // a version of the Simple protocol that doesn't declare "ack" one-way
    Protocol protocol = new Protocol("Simple", "org.apache.aingle.test");
    Protocol.Message message = protocol.createMessage("ack", null, new LinkedHashMap<String, String>(),
        Schema.createRecord(new ArrayList<>()), Schema.create(Schema.Type.NULL), Schema.createUnion(new ArrayList<>()));
    protocol.getMessages().put("ack", message);

    // call a server over a stateless protocol that has a one-way "ack"
    GenericRequestor requestor = new GenericRequestor(protocol, createTransceiver());
    requestor.request("ack", new GenericData.Record(message.getRequest()));

    // make the request again, to better test handshakes w/ differing protocols
    requestor.request("ack", new GenericData.Record(message.getRequest()));
  }

}
