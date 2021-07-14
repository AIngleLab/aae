/*

 */

package org.apache.aingle.ipc.netty;

import java.net.InetSocketAddress;

import org.apache.aingle.TestProtocolSpecific;
import org.apache.aingle.ipc.Responder;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.Transceiver;

/**
 * Protocol test with Netty server and transceiver
 */
public class TestProtocolNetty extends TestProtocolSpecific {
  @Override
  public Server createServer(Responder testResponder) throws Exception {
    return new NettyServer(responder, new InetSocketAddress(0));
  }

  @Override
  public Transceiver createTransceiver() throws Exception {
    return new NettyTransceiver(new InetSocketAddress(server.getPort()), 2000);
  }

  @Override
  protected int getExpectedHandshakeCount() {
    return REPEATING;
  }
}
