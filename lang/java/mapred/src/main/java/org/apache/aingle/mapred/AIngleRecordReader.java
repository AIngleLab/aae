/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.RecordReader;

import org.apache.aingle.file.FileReader;
import org.apache.aingle.file.DataFileReader;

/** An {@link RecordReader} for AIngle data files. */
public class AIngleRecordReader<T> implements RecordReader<AIngleWrapper<T>, NullWritable> {

  private FileReader<T> reader;
  private long start;
  private long end;

  public AIngleRecordReader(JobConf job, FileSplit split) throws IOException {
    this(DataFileReader.openReader(new FsInput(split.getPath(), job),
        AIngleJob.createInputDataModel(job).createDatumReader(AIngleJob.getInputSchema(job))), split);
  }

  protected AIngleRecordReader(FileReader<T> reader, FileSplit split) throws IOException {
    this.reader = reader;
    reader.sync(split.getStart()); // sync to start
    this.start = reader.tell();
    this.end = split.getStart() + split.getLength();
  }

  @Override
  public AIngleWrapper<T> createKey() {
    return new AIngleWrapper<>(null);
  }

  @Override
  public NullWritable createValue() {
    return NullWritable.get();
  }

  @Override
  public boolean next(AIngleWrapper<T> wrapper, NullWritable ignore) throws IOException {
    if (!reader.hasNext() || reader.pastSync(end))
      return false;
    wrapper.datum(reader.next(wrapper.datum()));
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
