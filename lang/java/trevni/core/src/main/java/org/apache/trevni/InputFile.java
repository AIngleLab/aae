/*

 */
package org.apache.trevni;

import java.io.File;
import java.io.FileInputStream;
import java.nio.channels.FileChannel;
import java.nio.ByteBuffer;
import java.io.IOException;

/** An {@link Input} for files. */
public class InputFile implements Input {

  private FileChannel channel;

  /** Construct for the given file. */
  public InputFile(File file) throws IOException {
    this.channel = new FileInputStream(file).getChannel();
  }

  @Override
  public long length() throws IOException {
    return channel.size();
  }

  @Override
  public int read(long position, byte[] b, int start, int len) throws IOException {
    return channel.read(ByteBuffer.wrap(b, start, len), position);
  }

  @Override
  public void close() throws IOException {
    channel.close();
  }

}
