/*

 */
package org.apache.aingle.file;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileDescriptor;
import java.io.IOException;

/** A {@link FileInputStream} that implements {@link SeekableInput}. */
public class SeekableFileInput extends FileInputStream implements SeekableInput {

  public SeekableFileInput(File file) throws IOException {
    super(file);
  }

  public SeekableFileInput(FileDescriptor fd) throws IOException {
    super(fd);
  }

  @Override
  public void seek(long p) throws IOException {
    getChannel().position(p);
  }

  @Override
  public long tell() throws IOException {
    return getChannel().position();
  }

  @Override
  public long length() throws IOException {
    return getChannel().size();
  }
}
