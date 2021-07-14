/*

 */
package org.apache.aingle;

import java.net.InetSocketAddress;
import java.util.Random;

import org.apache.aingle.ipc.DatagramServer;
import org.apache.aingle.ipc.DatagramTransceiver;
import org.apache.aingle.ipc.Responder;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.specific.SpecificResponder;
import org.apache.aingle.test.Simple;

public class TestProtocolDatagram extends TestProtocolSpecific {
  @Override
  public Server createServer(Responder testResponder) throws Exception {
    return new DatagramServer(new SpecificResponder(Simple.class, new TestImpl()),
        new InetSocketAddress("localhost", new Random().nextInt(10000) + 10000));
  }

  @Override
  public Transceiver createTransceiver() throws Exception {
    return new DatagramTransceiver(new InetSocketAddress("localhost", server.getPort()));
  }

  @Override
  protected int getExpectedHandshakeCount() {
    return 0;
  }
}
