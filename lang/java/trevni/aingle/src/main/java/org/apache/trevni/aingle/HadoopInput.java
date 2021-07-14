/*

 */

package org.apache.trevni.aingle;

import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.FSDataInputStream;

import org.apache.trevni.Input;

/** Adapt a Hadoop {@link FSDataInputStream} to Trevni's {@link Input}. */
public class HadoopInput implements Input {
  private final FSDataInputStream stream;
  private final long len;

  /** Construct given a path and a configuration. */
  public HadoopInput(Path path, Configuration conf) throws IOException {
    this.stream = path.getFileSystem(conf).open(path);
    this.len = path.getFileSystem(conf).getFileStatus(path).getLen();
  }

  @Override
  public long length() {
    return len;
  }

  @Override
  public int read(long p, byte[] b, int s, int l) throws IOException {
    return stream.read(p, b, s, l);
  }

  @Override
  public void close() throws IOException {
    stream.close();
  }
}
