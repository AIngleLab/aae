/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.mapred.AIngleKey;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;

/**
 * An {@link org.apache.hadoop.mapreduce.InputFormat} for Trevni files.
 *
 * This implement was modeled off
 * {@link org.apache.aingle.mapreduce.AIngleKeyInputFormat} to allow for easy
 * transition
 *
 * A MapReduce InputFormat that can handle Trevni container files.
 *
 * <p>
 * Keys are AIngleKey wrapper objects that contain the Trevni data. Since Trevni
 * container files store only records (not key/value pairs), the value from this
 * InputFormat is a NullWritable.
 * </p>
 *
 * <p>
 * A subset schema to be read may be specified with
 * {@link org.apache.aingle.mapreduce.AIngleJob#setInputKeySchema}.
 */
public class AIngleTrevniKeyInputFormat<T> extends FileInputFormat<AIngleKey<T>, NullWritable> {

  @Override
  public RecordReader<AIngleKey<T>, NullWritable> createRecordReader(InputSplit split, TaskAttemptContext context)
      throws IOException, InterruptedException {

    return new AIngleTrevniKeyRecordReader<>();
  }

}
