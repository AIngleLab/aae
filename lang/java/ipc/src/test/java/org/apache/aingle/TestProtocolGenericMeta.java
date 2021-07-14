/*

 */
package org.apache.aingle;

import java.net.InetSocketAddress;

import org.apache.aingle.ipc.Responder;
import org.apache.aingle.ipc.SocketServer;
import org.apache.aingle.ipc.SocketTransceiver;
import org.apache.aingle.ipc.generic.GenericRequestor;
import org.junit.Before;

public class TestProtocolGenericMeta extends TestProtocolGeneric {

  @Before
  @Override
  public void testStartServer() throws Exception {
    if (server != null)
      return;
    Responder responder = new TestResponder();
    responder.addRPCPlugin(new RPCMetaTestPlugin("key1"));
    responder.addRPCPlugin(new RPCMetaTestPlugin("key2"));
    server = new SocketServer(responder, new InetSocketAddress(0));
    server.start();

    client = new SocketTransceiver(new InetSocketAddress(server.getPort()));
    requestor = new GenericRequestor(PROTOCOL, client);
    requestor.addRPCPlugin(new RPCMetaTestPlugin("key1"));
    requestor.addRPCPlugin(new RPCMetaTestPlugin("key2"));
  }
}
