/*

 */
package org.apache.aingle.tool;

import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URI;

import org.apache.trevni.Input;
import org.apache.trevni.aingle.HadoopInput;
import org.apache.trevni.InputFile;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

/** Static utility methods for tools. */
class TrevniUtil {

  static Input input(String filename) throws IOException {
    if (filename.startsWith("hdfs://")) {
      return new HadoopInput(new Path(filename), new Configuration());
    } else {
      return new InputFile(new File(filename));
    }
  }

  /**
   * Returns stdin if filename is "-", else opens the local or HDFS file and
   * returns an InputStream for it.
   * 
   * @throws IOException
   */
  static InputStream input(String filename, InputStream stdin) throws IOException {
    if (filename.equals("-"))
      return new BufferedInputStream(stdin);
    else if (filename.startsWith("hdfs://")) {
      FileSystem fs = FileSystem.get(URI.create(filename), new Configuration());
      return new BufferedInputStream(fs.open(new Path(filename)));
    } else {
      return new BufferedInputStream(new FileInputStream(new File(filename)));
    }
  }

  /**
   * Returns stdout if filename is "-", else opens the local or HDFS file and
   * returns an OutputStream for it.
   * 
   * @throws IOException
   */
  static OutputStream output(String filename, OutputStream stdout) throws IOException {
    if (filename.equals("-"))
      return new BufferedOutputStream(stdout);
    else if (filename.startsWith("hdfs://")) {
      FileSystem fs = FileSystem.get(URI.create(filename), new Configuration());
      return new BufferedOutputStream(fs.create(new Path(filename)));
    } else {
      return new BufferedOutputStream(new FileOutputStream(new File(filename)));
    }
  }

}
