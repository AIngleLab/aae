/*

 */
package org.apache.aingle;

import java.net.InetSocketAddress;

import org.apache.aingle.ipc.Requestor;
import org.apache.aingle.ipc.Responder;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.SocketServer;
import org.apache.aingle.ipc.SocketTransceiver;
import org.apache.aingle.ipc.Transceiver;

public class TestProtocolSpecificMeta extends TestProtocolSpecific {

  @Override
  public Server createServer(Responder testResponder) throws Exception {
    responder.addRPCPlugin(new RPCMetaTestPlugin("key1"));
    responder.addRPCPlugin(new RPCMetaTestPlugin("key2"));
    return new SocketServer(responder, new InetSocketAddress(0));
  }

  @Override
  public Transceiver createTransceiver() throws Exception {
    return new SocketTransceiver(new InetSocketAddress(server.getPort()));
  }

  public void addRpcPlugins(Requestor req) {
    req.addRPCPlugin(new RPCMetaTestPlugin("key1"));
    req.addRPCPlugin(new RPCMetaTestPlugin("key2"));
  }
}
