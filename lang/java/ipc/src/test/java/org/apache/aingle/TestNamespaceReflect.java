/*

 */
package org.apache.aingle;

import org.apache.aingle.ipc.SocketServer;
import org.apache.aingle.ipc.SocketTransceiver;
import org.apache.aingle.ipc.reflect.ReflectRequestor;
import org.apache.aingle.ipc.reflect.ReflectResponder;
import org.apache.aingle.test.namespace.TestNamespace;
import org.junit.Before;

import java.net.InetSocketAddress;

public class TestNamespaceReflect extends TestNamespaceSpecific {

  @Before
  @Override
  public void testStartServer() throws Exception {
    if (server != null)
      return;
    server = new SocketServer(new ReflectResponder(TestNamespace.class, new TestImpl()), new InetSocketAddress(0));
    server.start();
    client = new SocketTransceiver(new InetSocketAddress(server.getPort()));
    proxy = ReflectRequestor.getClient(TestNamespace.class, client);
  }

}
