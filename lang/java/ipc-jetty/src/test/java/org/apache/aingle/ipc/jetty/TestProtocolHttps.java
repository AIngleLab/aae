/*

 */
package org.apache.aingle.ipc.jetty;

import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.Responder;
import org.apache.aingle.TestProtocolSpecific;
import org.apache.aingle.ipc.HttpTransceiver;
import org.eclipse.jetty.server.SslConnectionFactory;
import org.eclipse.jetty.util.ssl.SslContextFactory;

import java.net.URL;

public class TestProtocolHttps extends TestProtocolSpecific {

  @Override
  public Server createServer(Responder testResponder) throws Exception {
    System.setProperty("javax.net.ssl.keyStore", "src/test/keystore");
    System.setProperty("javax.net.ssl.keyStorePassword", "aingletest");
    System.setProperty("javax.net.ssl.password", "aingletest");
    System.setProperty("javax.net.ssl.trustStore", "src/test/truststore");
    System.setProperty("javax.net.ssl.trustStorePassword", "aingletest");
    SslConnectionFactory connectionFactory = new SslConnectionFactory("HTTP/1.1");
    SslContextFactory sslContextFactory = connectionFactory.getSslContextFactory();

    sslContextFactory.setKeyStorePath(System.getProperty("javax.net.ssl.keyStore"));
    sslContextFactory.setKeyManagerPassword(System.getProperty("javax.net.ssl.password"));
    sslContextFactory.setKeyStorePassword(System.getProperty("javax.net.ssl.keyStorePassword"));
    sslContextFactory.setNeedClientAuth(false);
    return new HttpServer(testResponder, connectionFactory, "localhost", 18443);
  }

  @Override
  public Transceiver createTransceiver() throws Exception {
    return new HttpTransceiver(new URL("https://localhost:" + server.getPort() + "/"));
  }

  protected int getExpectedHandshakeCount() {
    return REPEATING;
  }

}
