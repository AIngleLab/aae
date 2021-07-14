/*

 */

package org.apache.aingle.mapred.tether;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.RecordReader;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.mapred.AIngleJob;
import org.apache.aingle.mapred.FsInput;

class TetherRecordReader implements RecordReader<TetherData, NullWritable> {

  private FsInput in;
  private DataFileReader reader;
  private long start;
  private long end;

  public TetherRecordReader(JobConf job, FileSplit split) throws IOException {
    this.in = new FsInput(split.getPath(), job);
    this.reader = new DataFileReader<>(in, new GenericDatumReader<>());

    reader.sync(split.getStart()); // sync to start
    this.start = in.tell();
    this.end = split.getStart() + split.getLength();

    job.set(AIngleJob.INPUT_SCHEMA, reader.getSchema().toString());
  }

  public Schema getSchema() {
    return reader.getSchema();
  }

  @Override
  public TetherData createKey() {
    return new TetherData();
  }

  @Override
  public NullWritable createValue() {
    return NullWritable.get();
  }

  @Override
  public boolean next(TetherData data, NullWritable ignore) throws IOException {
    if (!reader.hasNext() || reader.pastSync(end))
      return false;
    data.buffer(reader.nextBlock());
    data.count((int) reader.getBlockCount());
    return true;
  }

  @Override
  public float getProgress() throws IOException {
    if (end == start) {
      return 0.0f;
    } else {
      return Math.min(1.0f, (in.tell() - start) / (float) (end - start));
    }
  }

  @Override
  public long getPos() throws IOException {
    return in.tell();
  }

  @Override
  public void close() throws IOException {
    reader.close();
  }
}
