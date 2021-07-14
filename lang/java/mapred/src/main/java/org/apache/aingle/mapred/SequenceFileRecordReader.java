/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileSplit;

/** A {@link org.apache.hadoop.mapred.RecordReader} for sequence files. */
public class SequenceFileRecordReader<K, V> extends AIngleRecordReader<Pair<K, V>> {

  public SequenceFileRecordReader(JobConf job, FileSplit split) throws IOException {
    super(new SequenceFileReader<>(split.getPath().toUri(), job), split);
  }

}
