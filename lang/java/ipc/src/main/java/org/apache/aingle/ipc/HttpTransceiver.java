/*

 */

package org.apache.aingle.ipc;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.EOFException;
import java.net.Proxy;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.net.URL;
import java.net.HttpURLConnection;

/** An HTTP-based {@link Transceiver} implementation. */
public class HttpTransceiver extends Transceiver {
  static final String CONTENT_TYPE = "aingle/binary";

  private URL url;
  private Proxy proxy;
  private HttpURLConnection connection;
  private int timeout;

  public HttpTransceiver(URL url) {
    this.url = url;
  }

  public HttpTransceiver(URL url, Proxy proxy) {
    this(url);
    this.proxy = proxy;
  }

  /** Set the connect and read timeouts, in milliseconds. */
  public void setTimeout(int timeout) {
    this.timeout = timeout;
  }

  @Override
  public String getRemoteName() {
    return this.url.toString();
  }

  @Override
  public synchronized List<ByteBuffer> readBuffers() throws IOException {
    try (InputStream in = connection.getInputStream()) {
      return readBuffers(in);
    }
  }

  @Override
  public synchronized void writeBuffers(List<ByteBuffer> buffers) throws IOException {
    if (proxy == null)
      connection = (HttpURLConnection) url.openConnection();
    else
      connection = (HttpURLConnection) url.openConnection(proxy);

    connection.setRequestMethod("POST");
    connection.setRequestProperty("Content-Type", CONTENT_TYPE);
    connection.setRequestProperty("Content-Length", Integer.toString(getLength(buffers)));
    connection.setDoOutput(true);
    connection.setReadTimeout(timeout);
    connection.setConnectTimeout(timeout);

    try (OutputStream out = connection.getOutputStream()) {
      writeBuffers(buffers, out);
    }
  }

  static int getLength(List<ByteBuffer> buffers) {
    int length = 0;
    for (ByteBuffer buffer : buffers) {
      length += 4;
      length += buffer.remaining();
    }
    length += 4;
    return length;
  }

  static List<ByteBuffer> readBuffers(InputStream in) throws IOException {
    List<ByteBuffer> buffers = new ArrayList<>();
    while (true) {
      int length = (in.read() << 24) + (in.read() << 16) + (in.read() << 8) + in.read();
      if (length == 0) { // end of buffers
        return buffers;
      }
      ByteBuffer buffer = ByteBuffer.allocate(length);
      while (buffer.hasRemaining()) {
        int p = buffer.position();
        int i = in.read(buffer.array(), p, buffer.remaining());
        if (i < 0)
          throw new EOFException("Unexpected EOF");
        ((Buffer) buffer).position(p + i);
      }
      ((Buffer) buffer).flip();
      buffers.add(buffer);
    }
  }

  static void writeBuffers(List<ByteBuffer> buffers, OutputStream out) throws IOException {
    for (ByteBuffer buffer : buffers) {
      writeLength(buffer.limit(), out); // length-prefix
      out.write(buffer.array(), buffer.position(), buffer.remaining());
      ((Buffer) buffer).position(buffer.limit());
    }
    writeLength(0, out); // null-terminate
  }

  private static void writeLength(int length, OutputStream out) throws IOException {
    out.write(0xff & (length >>> 24));
    out.write(0xff & (length >>> 16));
    out.write(0xff & (length >>> 8));
    out.write(0xff & length);
  }
}
