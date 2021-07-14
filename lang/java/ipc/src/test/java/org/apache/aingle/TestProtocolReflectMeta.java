/*

 */
package org.apache.aingle;

import org.apache.aingle.ipc.SocketServer;
import org.apache.aingle.ipc.SocketTransceiver;
import org.apache.aingle.ipc.reflect.ReflectRequestor;
import org.apache.aingle.ipc.reflect.ReflectResponder;
import org.junit.Before;

import java.net.InetSocketAddress;

public class TestProtocolReflectMeta extends TestProtocolReflect {

  @Before
  @Override
  public void testStartServer() throws Exception {
    if (server != null)
      return;
    ReflectResponder rresp = new ReflectResponder(Simple.class, new TestImpl());
    rresp.addRPCPlugin(new RPCMetaTestPlugin("key1"));
    rresp.addRPCPlugin(new RPCMetaTestPlugin("key2"));
    server = new SocketServer(rresp, new InetSocketAddress(0));
    server.start();

    client = new SocketTransceiver(new InetSocketAddress(server.getPort()));
    ReflectRequestor requestor = new ReflectRequestor(Simple.class, client);
    requestor.addRPCPlugin(new RPCMetaTestPlugin("key1"));
    requestor.addRPCPlugin(new RPCMetaTestPlugin("key2"));
    proxy = ReflectRequestor.getClient(Simple.class, requestor);
  }

}
