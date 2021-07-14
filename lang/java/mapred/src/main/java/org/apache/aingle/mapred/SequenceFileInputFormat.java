/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.RecordReader;

/** An {@link org.apache.hadoop.mapred.InputFormat} for sequence files. */
public class SequenceFileInputFormat<K, V> extends FileInputFormat<AIngleWrapper<Pair<K, V>>, NullWritable> {

  @Override
  public RecordReader<AIngleWrapper<Pair<K, V>>, NullWritable> getRecordReader(InputSplit split, JobConf job,
      Reporter reporter) throws IOException {
    reporter.setStatus(split.toString());
    return new SequenceFileRecordReader<>(job, (FileSplit) split);
  }

}
