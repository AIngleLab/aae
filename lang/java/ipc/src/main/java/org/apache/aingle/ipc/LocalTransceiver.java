/*

 */
package org.apache.aingle.ipc;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;

/** Implementation of IPC that remains in process. */
public class LocalTransceiver extends Transceiver {
  private Responder responder;

  public LocalTransceiver(Responder responder) {
    this.responder = responder;
  }

  @Override
  public String getRemoteName() {
    return "local";
  }

  @Override
  public List<ByteBuffer> transceive(List<ByteBuffer> request) throws IOException {
    return responder.respond(request);
  }

  @Override
  public List<ByteBuffer> readBuffers() throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeBuffers(List<ByteBuffer> buffers) throws IOException {
    throw new UnsupportedOperationException();
  }
}
