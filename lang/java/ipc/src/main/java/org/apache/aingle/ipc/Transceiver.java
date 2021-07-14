/*

 */

package org.apache.aingle.ipc;

import java.io.Closeable;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.aingle.Protocol;

/** Base transport class used by {@link Requestor}. */
public abstract class Transceiver implements Closeable {
  private final ReentrantLock channelLock = new ReentrantLock();

  public abstract String getRemoteName() throws IOException;

  /**
   * Acquires an exclusive lock on the transceiver's channel.
   */
  public void lockChannel() {
    channelLock.lock();
  }

  /**
   * Releases the lock on the transceiver's channel if held by the calling thread.
   */
  public void unlockChannel() {
    if (channelLock.isHeldByCurrentThread()) {
      channelLock.unlock();
    }
  }

  /**
   * Called by {@link Requestor#request(String,Object)} for two-way messages. By
   * default calls {@link #writeBuffers(List)} followed by {@link #readBuffers()}.
   */
  public List<ByteBuffer> transceive(List<ByteBuffer> request) throws IOException {
    lockChannel();
    try {
      writeBuffers(request);
      return readBuffers();
    } finally {
      unlockChannel();
    }
  }

  /**
   * Called by {@link Requestor#request(String,Object,Callback)} for two-way
   * messages using callbacks.
   */
  public void transceive(List<ByteBuffer> request, Callback<List<ByteBuffer>> callback) throws IOException {
    // The default implementation works synchronously
    try {
      List<ByteBuffer> response = transceive(request);
      callback.handleResult(response);
    } catch (IOException e) {
      callback.handleError(e);
    }
  }

  /** Called by the default definition of {@link #transceive(List)}. */
  public abstract List<ByteBuffer> readBuffers() throws IOException;

  /** Called by {@link Requestor#request(String,Object)} for one-way messages. */
  public abstract void writeBuffers(List<ByteBuffer> buffers) throws IOException;

  /**
   * True if a handshake has been completed for this connection. Used to determine
   * whether a handshake need be completed prior to a one-way message. Requests
   * and responses are always prefixed by handshakes, but one-way messages. If the
   * first request sent over a connection is one-way, then a handshake-only
   * response is returned. Subsequent one-way messages over the connection will
   * have no response data sent. Returns false by default.
   */
  public boolean isConnected() {
    return false;
  }

  /**
   * Called with the remote protocol when a handshake has been completed. After
   * this has been called and while a connection is maintained,
   * {@link #isConnected()} should return true and #getRemote() should return this
   * protocol. Does nothing by default.
   */
  public void setRemote(Protocol protocol) {
  }

  /**
   * Returns the protocol passed to {@link #setRemote(Protocol)}. Throws
   * IllegalStateException by default.
   */
  public Protocol getRemote() {
    throw new IllegalStateException("Not connected.");
  }

  @Override
  public void close() throws IOException {
  }
}
