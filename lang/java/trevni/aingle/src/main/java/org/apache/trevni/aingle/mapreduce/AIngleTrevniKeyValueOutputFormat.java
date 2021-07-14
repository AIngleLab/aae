/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

/**
 * An {@link org.apache.hadoop.mapreduce.OutputFormat} that writes AIngle data to
 * Trevni files.
 *
 * This implement was modeled off
 * {@link org.apache.aingle.mapreduce.AIngleKeyValueOutputFormat} to allow for easy
 * transition
 *
 * * FileOutputFormat for writing Trevni container files of key/value pairs.
 *
 * <p>
 * Since Trevni container files can only contain records (not key/value pairs),
 * this output format puts the key and value into an AIngle generic record with
 * two fields, named 'key' and 'value'.
 * </p>
 *
 * <p>
 * The keys and values given to this output format may be AIngle objects wrapped
 * in <code>AIngleKey</code> or <code>AIngleValue</code> objects. The basic Writable
 * types are also supported (e.g., IntWritable, Text); they will be converted to
 * their corresponding AIngle types.
 * </p>
 *
 * @param <K> The type of key. If an AIngle type, it must be wrapped in an
 *            <code>AIngleKey</code>.
 * @param <V> The type of value. If an AIngle type, it must be wrapped in an
 *            <code>AIngleValue</code>.
 *
 *            <p>
 *            Writes a directory of files per task, each comprising a single
 *            filesystem block. To reduce the number of files, increase the
 *            default filesystem block size for the job. Each task also requires
 *            enough memory to buffer a filesystem block.
 */
public class AIngleTrevniKeyValueOutputFormat<K, V> extends FileOutputFormat<AIngleKey<K>, AIngleValue<V>> {

  /** {@inheritDoc} */
  @Override
  public RecordWriter<AIngleKey<K>, AIngleValue<V>> getRecordWriter(TaskAttemptContext context)
      throws IOException, InterruptedException {

    return new AIngleTrevniKeyValueRecordWriter<>(context);
  }
}
