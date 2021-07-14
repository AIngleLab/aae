/*

 */
package org.apache.aingle.ipc.jetty;

import org.apache.aingle.ipc.HttpTransceiver;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.specific.SpecificRequestor;
import org.apache.aingle.ipc.specific.SpecificResponder;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.Random;

import org.apache.aingle.test.BulkData;

public class TestBulkData {
  private static final long COUNT = Integer.parseInt(System.getProperty("test.count", "10"));
  private static final int SIZE = Integer.parseInt(System.getProperty("test.size", "65536"));

  private static final ByteBuffer DATA = ByteBuffer.allocate(SIZE);
  static {
    Random rand = new Random();
    DATA.limit(DATA.capacity());
    DATA.position(0);
    rand.nextBytes(DATA.array());
  }

  public static class BulkDataImpl implements BulkData {

    @Override
    public ByteBuffer read() {
      return DATA.duplicate();
    }

    @Override
    public void write(ByteBuffer data) {
      Assert.assertEquals(SIZE, data.remaining());
    }
  }

  private static Server server;
  private static BulkData proxy;

  @Before
  public void startServer() throws Exception {
    if (server != null)
      return;
    server = new HttpServer(new SpecificResponder(BulkData.class, new BulkDataImpl()), 0);
    server.start();
    Transceiver client = new HttpTransceiver(new URL("http://127.0.0.1:" + server.getPort() + "/"));
    proxy = SpecificRequestor.getClient(BulkData.class, client);
  }

  @Test
  public void testRead() throws IOException {
    for (int i = 0; i < COUNT; i++)
      Assert.assertEquals(SIZE, proxy.read().remaining());
  }

  @Test
  public void testWrite() throws IOException {
    for (int i = 0; i < COUNT; i++)
      proxy.write(DATA.duplicate());
  }

  @AfterClass
  public static void stopServer() throws Exception {
    server.close();
  }

  public static void main(String[] args) throws Exception {
    TestBulkData test = new TestBulkData();
    test.startServer();
    System.out.println("READ");
    long start = System.currentTimeMillis();
    test.testRead();
    printStats(start);
    System.out.println("WRITE");
    start = System.currentTimeMillis();
    test.testWrite();
    printStats(start);
    test.stopServer();
  }

  private static void printStats(long start) {
    double seconds = (System.currentTimeMillis() - start) / 1000.0;
    System.out.println("seconds = " + (int) seconds);
    System.out.println("requests/second = " + (int) (COUNT / seconds));
    double megabytes = (COUNT * SIZE) / (1024 * 1024.0);
    System.out.println("MB = " + (int) megabytes);
    System.out.println("MB/second = " + (int) (megabytes / seconds));
  }

}
