/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.mapred.AIngleKey;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

/**
 * An {@link org.apache.hadoop.mapreduce.OutputFormat} that writes AIngle data to
 * Trevni files.
 *
 * This implement was modeled off
 * {@link org.apache.aingle.mapreduce.AIngleKeyOutputFormat} to allow for easy
 * transition
 *
 * FileOutputFormat for writing Trevni container files.
 *
 * <p>
 * Since Trevni container files only contain records (not key/value pairs), this
 * output format ignores the value.
 * </p>
 *
 * @param <T> The (java) type of the Trevni data to write.
 *
 *            <p>
 *            Writes a directory of files per task, each comprising a single
 *            filesystem block. To reduce the number of files, increase the
 *            default filesystem block size for the job. Each task also requires
 *            enough memory to buffer a filesystem block.
 */
public class AIngleTrevniKeyOutputFormat<T> extends FileOutputFormat<AIngleKey<T>, NullWritable> {

  @Override
  public RecordWriter<AIngleKey<T>, NullWritable> getRecordWriter(TaskAttemptContext context)
      throws IOException, InterruptedException {

    return new AIngleTrevniKeyRecordWriter<>(context);
  }
}
