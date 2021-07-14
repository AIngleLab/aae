/*

 */

package org.apache.aingle.ipc;

import java.util.List;
import java.util.ArrayList;
import java.io.IOException;
import java.net.SocketAddress;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A datagram-based {@link Transceiver} implementation. This uses a simple,
 * non-standard wire protocol and is not intended for production services.
 */
public class DatagramTransceiver extends Transceiver {
  private static final Logger LOG = LoggerFactory.getLogger(DatagramTransceiver.class);

  private static final int MAX_SIZE = 16 * 1024;

  private DatagramChannel channel;
  private SocketAddress remote;
  private ByteBuffer buffer = ByteBuffer.allocate(MAX_SIZE);

  @Override
  public String getRemoteName() {
    return remote.toString();
  }

  public DatagramTransceiver(SocketAddress remote) throws IOException {
    this(DatagramChannel.open());
    this.remote = remote;
  }

  public DatagramTransceiver(DatagramChannel channel) {
    this.channel = channel;
  }

  @Override
  public synchronized List<ByteBuffer> readBuffers() throws IOException {
    ((Buffer) buffer).clear();
    remote = channel.receive(buffer);
    LOG.info("received from " + remote);
    ((Buffer) buffer).flip();
    List<ByteBuffer> buffers = new ArrayList<>();
    while (true) {
      int length = buffer.getInt();
      if (length == 0) { // end of buffers
        return buffers;
      }
      ByteBuffer chunk = buffer.slice(); // use data without copying
      ((Buffer) chunk).limit(length);
      ((Buffer) buffer).position(buffer.position() + length);
      buffers.add(chunk);
    }
  }

  @Override
  public synchronized void writeBuffers(List<ByteBuffer> buffers) throws IOException {
    ((Buffer) buffer).clear();
    for (ByteBuffer b : buffers) {
      buffer.putInt(b.remaining());
      buffer.put(b); // copy data. sigh.
    }
    buffer.putInt(0);
    ((Buffer) buffer).flip();
    channel.send(buffer, remote);
    LOG.info("sent to " + remote);
  }

}
