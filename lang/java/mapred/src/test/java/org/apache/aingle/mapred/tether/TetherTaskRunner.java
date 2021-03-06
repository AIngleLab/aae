/*

 */

package org.apache.aingle.mapred.tether;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.aingle.ipc.SaslSocketServer;
import org.apache.aingle.ipc.specific.SpecificResponder;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.jetty.HttpServer;

/**
 * Java implementation of a tether executable. Useless except for testing, since
 * it's already possible to write Java MapReduce programs without tethering.
 * Also serves as an example of how a framework may be implemented.
 */
public class TetherTaskRunner implements InputProtocol {
  static final Logger LOG = LoggerFactory.getLogger(TetherTaskRunner.class);

  private Server inputServer;
  private TetherTask task;

  public TetherTaskRunner(TetherTask task) throws IOException {
    this.task = task;

    // determine what protocol we are using
    String protocol = System.getenv("AINGLE_TETHER_PROTOCOL");
    if (protocol == null) {
      throw new RuntimeException("AINGLE_TETHER_PROTOCOL env var is null");
    }

    protocol = protocol.trim().toLowerCase();

    TetheredProcess.Protocol proto;
    if (protocol.equals("http")) {
      LOG.info("Use HTTP protocol");
      proto = TetheredProcess.Protocol.HTTP;
    } else if (protocol.equals("sasl")) {
      LOG.info("Use SASL protocol");
      proto = TetheredProcess.Protocol.SASL;
    } else {
      throw new RuntimeException("AINGLE_TETHER_PROTOCOL=" + protocol + " but this protocol is unsupported");
    }

    InetSocketAddress iaddress = new InetSocketAddress(0);

    switch (proto) {
    case SASL:
      // start input server
      this.inputServer = new SaslSocketServer(new SpecificResponder(InputProtocol.class, this), iaddress);
      LOG.info("Started SaslSocketServer on port:" + iaddress.getPort());
      break;

    case HTTP:
      this.inputServer = new HttpServer(new SpecificResponder(InputProtocol.class, this), iaddress.getPort());

      LOG.info("Started HttpServer on port:" + iaddress.getPort());
      break;
    }

    inputServer.start();

    // open output to parent
    task.open(inputServer.getPort());
  }

  @Override
  public void configure(TaskType taskType, String inSchema, String outSchema) {
    LOG.info("got configure");
    task.configure(taskType, inSchema, outSchema);
  }

  @Override
  public synchronized void input(ByteBuffer data, long count) {
    task.input(data, count);
  }

  @Override
  public void partitions(int partitions) {
    task.partitions(partitions);
  }

  @Override
  public void abort() {
    LOG.info("got abort");
    close();
  }

  @Override
  public synchronized void complete() {
    LOG.info("got input complete");
    task.complete();
  }

  /** Wait for task to complete. */
  public void join() throws InterruptedException {
    LOG.info("TetherTaskRunner: Start join.");
    inputServer.join();
    LOG.info("TetherTaskRunner: Finish join.");
  }

  private void close() {
    LOG.info("Closing the task");
    task.close();
    LOG.info("Finished closing the task.");
    if (inputServer != null)
      inputServer.close();
  }
}
