/*

 */

package org.apache.aingle.ipc;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.URI;

import org.slf4j.LoggerFactory;

/** IPC utilities, including client and server factories. */
public class Ipc {
  private Ipc() {
  } // no public ctor

  static boolean warned = false;

  /** Create a client {@link Transceiver} connecting to the provided URI. */
  public static Transceiver createTransceiver(URI uri) throws IOException {
    if ("http".equals(uri.getScheme()))
      return new HttpTransceiver(uri.toURL());
    else if ("aingle".equals(uri.getScheme()))
      return new SaslSocketTransceiver(new InetSocketAddress(uri.getHost(), uri.getPort()));
    else
      throw new IOException("unknown uri scheme: " + uri);
  }

  /**
   * Create a {@link Server} listening at the named URI using the provided
   * responder.
   */
  public static Server createServer(Responder responder, URI uri) throws IOException {
    if ("aingle".equals(uri.getScheme())) {
      return new SaslSocketServer(responder, new InetSocketAddress(uri.getHost(), uri.getPort()));
    } else if ("http".equals(uri.getScheme())) {
      if (!warned) {
        LoggerFactory.getLogger(Ipc.class)
            .error("Using Ipc.createServer to create http instances is deprecated.  Create "
                + " an instance of org.apache.aingle.ipc.jetty.HttpServer directly.");
        warned = true;
      }
      try {
        Class<?> cls = Class.forName("org.apache.aingle.ipc.jetty.HttpServer");
        return (Server) cls.getConstructor(Responder.class, Integer.TYPE).newInstance(responder, uri.getPort());
      } catch (Throwable t) {
        // ignore, exception will be thrown
      }
    }
    throw new IOException("unknown uri scheme: " + uri);
  }

}
