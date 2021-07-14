/*

 */
package org.apache.aingle.tool;

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.io.File;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.net.URI;

import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;

import org.apache.aingle.AIngleRemoteException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Protocol.Message;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.io.JsonEncoder;
import org.apache.aingle.ipc.Ipc;
import org.apache.aingle.ipc.Server;
import org.apache.aingle.ipc.generic.GenericResponder;

/**
 * Receives one RPC call and responds. (The moral equivalent of "netcat".)
 */
public class RpcReceiveTool implements Tool {
  private PrintStream out;
  private Object response;
  /** Used to communicate between server thread (responder) and run() */
  private CountDownLatch latch;
  private Message expectedMessage;
  Server server;

  @Override
  public String getName() {
    return "rpcreceive";
  }

  @Override
  public String getShortDescription() {
    return "Opens an RPC Server and listens for one message.";
  }

  private class SinkResponder extends GenericResponder {

    public SinkResponder(Protocol local) {
      super(local);
    }

    @Override
    public Object respond(Message message, Object request) throws AIngleRemoteException {
      if (!message.equals(expectedMessage)) {
        out.println(
            String.format("Expected message '%s' but received '%s'.", expectedMessage.getName(), message.getName()));
        latch.countDown();
        throw new IllegalArgumentException("Unexpected message.");
      }
      out.print(message.getName());
      out.print("\t");
      try {
        JsonEncoder jsonEncoder = EncoderFactory.get().jsonEncoder(message.getRequest(), out);
        GenericDatumWriter<Object> writer = new GenericDatumWriter<>(message.getRequest());
        writer.write(request, jsonEncoder);
        jsonEncoder.flush();
        out.flush();
      } catch (IOException e) {
        throw new RuntimeException(e);
      }
      out.println();
      new Thread(() -> {
        try {
          Thread.sleep(1000);
        } catch (InterruptedException e) {
        }
        latch.countDown();
      }).start();
      return response;
    }
  }

  @Override
  public int run(InputStream in, PrintStream out, PrintStream err, List<String> args) throws Exception {
    // Split up into two functions for easier testing.
    int r = run1(in, out, err, args);
    if (r != 0) {
      return r;
    }
    return run2(err);
  }

  int run1(InputStream in, PrintStream out, PrintStream err, List<String> args) throws Exception {
    OptionParser p = new OptionParser();
    OptionSpec<String> file = p.accepts("file", "Data file containing response datum.").withRequiredArg()
        .ofType(String.class);
    OptionSpec<String> data = p.accepts("data", "JSON-encoded response datum.").withRequiredArg().ofType(String.class);
    OptionSet opts = p.parse(args.toArray(new String[0]));
    args = (List<String>) opts.nonOptionArguments();

    if (args.size() != 3) {
      err.println("Usage: uri protocol_file message_name (-data d | -file f)");
      p.printHelpOn(err);
      return 1;
    }

    URI uri = new URI(args.get(0));
    Protocol protocol = Protocol.parse(new File(args.get(1)));
    String messageName = args.get(2);
    expectedMessage = protocol.getMessages().get(messageName);
    if (expectedMessage == null) {
      err.println(String.format("No message named '%s' found in protocol '%s'.", messageName, protocol));
      return 1;
    }
    if (data.value(opts) != null) {
      this.response = Util.jsonToGenericDatum(expectedMessage.getResponse(), data.value(opts));
    } else if (file.value(opts) != null) {
      this.response = Util.datumFromFile(expectedMessage.getResponse(), file.value(opts));
    } else {
      err.println("One of -data or -file must be specified.");
      return 1;
    }

    this.out = out;

    latch = new CountDownLatch(1);
    server = Ipc.createServer(new SinkResponder(protocol), uri);
    server.start();
    out.println("Port: " + server.getPort());
    return 0;
  }

  int run2(PrintStream err) throws InterruptedException {
    latch.await();
    err.println("Closing server.");
    server.close();
    return 0;
  }

}
