/*

 */
package org.apache.aingle.tool;

import org.apache.aingle.Protocol;
import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.ipc.HandshakeRequest;
import org.apache.aingle.ipc.HandshakeResponse;
import org.apache.aingle.ipc.Ipc;
import org.apache.aingle.ipc.MD5;
import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.specific.SpecificDatumWriter;
import org.apache.aingle.util.ByteBufferInputStream;
import org.apache.aingle.util.ByteBufferOutputStream;

import java.io.InputStream;
import java.io.PrintStream;
import java.net.URI;
import java.nio.ByteBuffer;
import java.util.LinkedHashMap;
import java.util.List;

/**
 * Tool to grab the protocol from a remote running service.
 */
public class RpcProtocolTool implements Tool {

  @Override
  public String getName() {
    return "rpcprotocol";
  }

  @Override
  public String getShortDescription() {
    return "Output the protocol of a RPC service";
  }

  @Override
  public int run(InputStream in, PrintStream out, PrintStream err, List<String> args) throws Exception {

    if (args.size() != 1) {
      err.println("Usage: uri");
      return 1;
    }

    URI uri = URI.create(args.get(0));

    try (Transceiver transceiver = Ipc.createTransceiver(uri)) {

      // write an empty HandshakeRequest
      HandshakeRequest rq = HandshakeRequest.newBuilder().setClientHash(new MD5(new byte[16]))
          .setServerHash(new MD5(new byte[16])).setClientProtocol(null).setMeta(new LinkedHashMap<>()).build();

      DatumWriter<HandshakeRequest> handshakeWriter = new SpecificDatumWriter<>(HandshakeRequest.class);

      ByteBufferOutputStream byteBufferOutputStream = new ByteBufferOutputStream();

      BinaryEncoder encoder = EncoderFactory.get().binaryEncoder(byteBufferOutputStream, null);

      handshakeWriter.write(rq, encoder);
      encoder.flush();

      // send it and get the response
      List<ByteBuffer> response = transceiver.transceive(byteBufferOutputStream.getBufferList());

      // parse the response
      ByteBufferInputStream byteBufferInputStream = new ByteBufferInputStream(response);

      DatumReader<HandshakeResponse> handshakeReader = new SpecificDatumReader<>(HandshakeResponse.class);

      HandshakeResponse handshakeResponse = handshakeReader.read(null,
          DecoderFactory.get().binaryDecoder(byteBufferInputStream, null));

      Protocol p = Protocol.parse(handshakeResponse.getServerProtocol());

      // finally output the protocol
      out.println(p.toString(true));

    }
    return 0;
  }
}
