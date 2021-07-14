/*

 */
package org.apache.aingle.ipc.netty;

import java.io.IOException;

import org.junit.BeforeClass;
import org.junit.Ignore;

import io.netty.handler.codec.compression.JdkZlibDecoder;
import io.netty.handler.codec.compression.JdkZlibEncoder;

public class TestNettyServerWithCompression extends TestNettyServer {

  @BeforeClass
  public static void initializeConnections() throws Exception {
    initializeConnections(ch -> {
      ch.pipeline().addFirst("deflater", new JdkZlibEncoder(6));
      ch.pipeline().addFirst("inflater", new JdkZlibDecoder());
    });
  }

  @Ignore
  @Override
  public void testBadRequest() throws IOException {
    // this tests in the base class needs to be skipped
    // as the decompression/compression algorithms will write the gzip header out
    // prior to the stream closing so the stream is not completely empty
  }
}
