/*

 */

package org.apache.aingle.ipc.netty;

import java.security.KeyStore;
import java.security.Security;
import java.security.cert.X509Certificate;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import org.junit.BeforeClass;

import io.netty.handler.ssl.SslHandler;

public class TestNettyServerWithSSL extends TestNettyServer {
  public static final String TEST_CERTIFICATE = "servercert.p12";
  public static final String TEST_CERTIFICATE_PASSWORD = "s3cret";

  @BeforeClass
  public static void initializeConnections() throws Exception {
    initializeConnections(ch -> {
      SSLEngine sslEngine = createServerSSLContext().createSSLEngine();
      sslEngine.setUseClientMode(false);
      SslHandler handler = new SslHandler(sslEngine, false);
      ch.pipeline().addLast("SSL", handler);
    }, ch -> {
      try {
        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(null, new TrustManager[] { new BogusTrustManager() }, null);
        SSLEngine sslEngine = sslContext.createSSLEngine();
        sslEngine.setUseClientMode(true);

        SslHandler handler = new SslHandler(sslEngine, false);
        ch.pipeline().addLast("SSL", handler);
      } catch (Exception e) {
        e.printStackTrace();
      }
    });
  }

  /**
   * Bogus trust manager accepting any certificate
   */
  private static class BogusTrustManager implements X509TrustManager {
    @Override
    public void checkClientTrusted(X509Certificate[] certs, String s) {
      // nothing
    }

    @Override
    public void checkServerTrusted(X509Certificate[] certs, String s) {
      // nothing
    }

    @Override
    public X509Certificate[] getAcceptedIssuers() {
      return new X509Certificate[0];
    }
  }

  private static SSLContext createServerSSLContext() {
    try {
      KeyStore ks = KeyStore.getInstance("PKCS12");
      ks.load(TestNettyServer.class.getResource(TEST_CERTIFICATE).openStream(),
          TEST_CERTIFICATE_PASSWORD.toCharArray());

      // Set up key manager factory to use our key store
      KeyManagerFactory kmf = KeyManagerFactory.getInstance(getAlgorithm());
      kmf.init(ks, TEST_CERTIFICATE_PASSWORD.toCharArray());

      SSLContext serverContext = SSLContext.getInstance("TLS");
      serverContext.init(kmf.getKeyManagers(), null, null);
      return serverContext;
    } catch (Exception e) {
      throw new Error("Failed to initialize the server-side SSLContext", e);
    }
  }

  private static String getAlgorithm() {
    String algorithm = Security.getProperty("ssl.KeyManagerFactory.algorithm");
    if (algorithm == null) {
      algorithm = "SunX509";
    }
    return algorithm;
  }
}
