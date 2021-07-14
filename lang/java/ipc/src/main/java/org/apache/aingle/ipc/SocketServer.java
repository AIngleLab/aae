/*

 */

package org.apache.aingle.ipc;

import java.io.IOException;
import java.io.EOFException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.aingle.AIngleRemoteException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Protocol.Message;
import org.apache.aingle.ipc.generic.GenericResponder;

/**
 * A socket-based server implementation. This uses a simple, non-standard wire
 * protocol and is not intended for production services.
 *
 * @deprecated use {@link SaslSocketServer} instead.
 */
@Deprecated
public class SocketServer extends Thread implements Server {
  private static final Logger LOG = LoggerFactory.getLogger(SocketServer.class);

  private Responder responder;
  private ServerSocketChannel channel;
  private ThreadGroup group;

  public SocketServer(Responder responder, SocketAddress addr) throws IOException {
    String name = "SocketServer on " + addr;

    this.responder = responder;
    this.group = new ThreadGroup(name);
    this.channel = ServerSocketChannel.open();

    channel.socket().bind(addr);

    setName(name);
    setDaemon(true);
  }

  @Override
  public int getPort() {
    return channel.socket().getLocalPort();
  }

  @Override
  public void run() {
    LOG.info("starting " + channel.socket().getInetAddress());
    try {
      while (true) {
        try {
          new Connection(channel.accept());
        } catch (ClosedChannelException e) {
          return;
        } catch (IOException e) {
          LOG.warn("unexpected error", e);
          throw new RuntimeException(e);
        }
      }
    } finally {
      LOG.info("stopping " + channel.socket().getInetAddress());
      try {
        channel.close();
      } catch (IOException e) {
      }
    }
  }

  @Override
  public void close() {
    this.interrupt();
    group.interrupt();
  }

  /**
   * Creates an appropriate {@link Transceiver} for this server. Returns a
   * {@link SocketTransceiver} by default.
   */
  protected Transceiver getTransceiver(SocketChannel channel) throws IOException {
    return new SocketTransceiver(channel);
  }

  private class Connection implements Runnable {

    SocketChannel channel;
    Transceiver xc;

    public Connection(SocketChannel channel) throws IOException {
      this.channel = channel;

      Thread thread = new Thread(group, this);
      thread.setName("Connection to " + channel.socket().getRemoteSocketAddress());
      thread.setDaemon(true);
      thread.start();
    }

    @Override
    public void run() {
      try {
        try {
          this.xc = getTransceiver(channel);
          while (true) {
            xc.writeBuffers(responder.respond(xc.readBuffers(), xc));
          }
        } catch (EOFException | ClosedChannelException e) {
        } finally {
          xc.close();
        }
      } catch (IOException e) {
        LOG.warn("unexpected error", e);
      }
    }

  }

  public static void main(String[] arg) throws Exception {
    Responder responder = new GenericResponder(Protocol.parse("{\"protocol\": \"X\"}")) {
      @Override
      public Object respond(Message message, Object request) throws Exception {
        throw new AIngleRemoteException("no messages!");
      }
    };
    SocketServer server = new SocketServer(responder, new InetSocketAddress(0));
    server.start();
    System.out.println("server started on port: " + server.getPort());
    server.join();
  }
}
