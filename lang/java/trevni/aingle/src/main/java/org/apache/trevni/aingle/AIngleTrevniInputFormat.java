/*

 */

package org.apache.trevni.aingle;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.RecordReader;

import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.mapred.AIngleJob;
import org.apache.aingle.mapred.AIngleWrapper;

/**
 * An {@link org.apache.hadoop.mapred.InputFormat} for Trevni files.
 *
 * <p>
 * A subset schema to be read may be specified with
 * {@link AIngleJob#setInputSchema(JobConf,Schema)}.
 */
public class AIngleTrevniInputFormat<T> extends FileInputFormat<AIngleWrapper<T>, NullWritable> {

  @Override
  protected boolean isSplitable(FileSystem fs, Path filename) {
    return false;
  }

  @Override
  protected FileStatus[] listStatus(JobConf job) throws IOException {
    List<FileStatus> result = new ArrayList<>();
    job.setBoolean("mapred.input.dir.recursive", true);
    for (FileStatus file : super.listStatus(job))
      if (file.getPath().getName().endsWith(AIngleTrevniOutputFormat.EXT))
        result.add(file);
    return result.toArray(new FileStatus[0]);
  }

  @Override
  public RecordReader<AIngleWrapper<T>, NullWritable> getRecordReader(InputSplit split, final JobConf job,
      Reporter reporter) throws IOException {
    final FileSplit file = (FileSplit) split;
    reporter.setStatus(file.toString());

    final AIngleColumnReader.Params params = new AIngleColumnReader.Params(new HadoopInput(file.getPath(), job));
    params.setModel(ReflectData.get());
    if (job.get(AIngleJob.INPUT_SCHEMA) != null)
      params.setSchema(AIngleJob.getInputSchema(job));

    return new RecordReader<AIngleWrapper<T>, NullWritable>() {
      private AIngleColumnReader<T> reader = new AIngleColumnReader<>(params);
      private float rows = reader.getRowCount();
      private long row;

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
        if (!reader.hasNext())
          return false;
        wrapper.datum(reader.next());
        row++;
        return true;
      }

      @Override
      public float getProgress() throws IOException {
        return row / rows;
      }

      @Override
      public long getPos() throws IOException {
        return row;
      }

      @Override
      public void close() throws IOException {
        reader.close();
      }

    };

  }

}
