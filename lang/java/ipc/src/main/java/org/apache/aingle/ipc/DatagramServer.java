/*

 */

package org.apache.aingle.ipc;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A datagram-based server implementation. This uses a simple, non-standard wire
 * protocol and is not intended for production services.
 */
public class DatagramServer extends Thread implements Server {
  private static final Logger LOG = LoggerFactory.getLogger(DatagramServer.class);

  private final Responder responder;
  private final DatagramChannel channel;
  private final Transceiver transceiver;

  public DatagramServer(Responder responder, SocketAddress addr) throws IOException {
    String name = "DatagramServer on " + addr;

    this.responder = responder;

    this.channel = DatagramChannel.open();
    channel.socket().bind(addr);

    this.transceiver = new DatagramTransceiver(channel);

    setName(name);
    setDaemon(true);
  }

  @Override
  public int getPort() {
    return channel.socket().getLocalPort();
  }

  @Override
  public void run() {
    while (true) {
      try {
        transceiver.writeBuffers(responder.respond(transceiver.readBuffers()));
      } catch (ClosedChannelException e) {
        return;
      } catch (IOException e) {
        LOG.warn("unexpected error", e);
        throw new RuntimeException(e);
      }
    }
  }

  @Override
  public void close() {
    this.interrupt();
  }

  public static void main(String[] arg) throws Exception {
    DatagramServer server = new DatagramServer(null, new InetSocketAddress(0));
    server.start();
    System.out.println("started");
    server.join();
  }

}
