/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.FileReader;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.RecordReader;

class AIngleAsTextRecordReader<T> implements RecordReader<Text, Text> {

  private FileReader<T> reader;
  private T datum;
  private long start;
  private long end;

  public AIngleAsTextRecordReader(JobConf job, FileSplit split) throws IOException {
    this(DataFileReader.openReader(new FsInput(split.getPath(), job), new GenericDatumReader<>()), split);
  }

  protected AIngleAsTextRecordReader(FileReader<T> reader, FileSplit split) throws IOException {
    this.reader = reader;
    reader.sync(split.getStart()); // sync to start
    this.start = reader.tell();
    this.end = split.getStart() + split.getLength();
  }

  @Override
  public Text createKey() {
    return new Text();
  }

  @Override
  public Text createValue() {
    return new Text();
  }

  @Override
  public boolean next(Text key, Text ignore) throws IOException {
    if (!reader.hasNext() || reader.pastSync(end))
      return false;
    datum = reader.next(datum);
    if (datum instanceof ByteBuffer) {
      ByteBuffer b = (ByteBuffer) datum;
      if (b.hasArray()) {
        int offset = b.arrayOffset();
        int start = b.position();
        int length = b.remaining();
        key.set(b.array(), offset + start, offset + start + length);
      } else {
        byte[] bytes = new byte[b.remaining()];
        b.duplicate().get(bytes);
        key.set(bytes);
      }
    } else {
      key.set(GenericData.get().toString(datum));
    }
    return true;
  }

  @Override
  public float getProgress() throws IOException {
    if (end == start) {
      return 0.0f;
    } else {
      return Math.min(1.0f, (getPos() - start) / (float) (end - start));
    }
  }

  @Override
  public long getPos() throws IOException {
    return reader.tell();
  }

  @Override
  public void close() throws IOException {
    reader.close();
  }
}
