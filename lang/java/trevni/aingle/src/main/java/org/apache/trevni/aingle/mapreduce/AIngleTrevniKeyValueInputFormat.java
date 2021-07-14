/*

 */
package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;

/**
 * An {@link org.apache.hadoop.mapreduce.InputFormat} for Trevni files.
 *
 * This implement was modeled off
 * {@link org.apache.aingle.mapreduce.AIngleKeyValueInputFormat} to allow for easy
 * transition
 *
 * <p>
 * A MapReduce InputFormat that reads from Trevni container files of key/value
 * generic records.
 *
 * <p>
 * Trevni container files that container generic records with the two fields
 * 'key' and 'value' are expected. The contents of the 'key' field will be used
 * as the job input key, and the contents of the 'value' field will be used as
 * the job output value.
 * </p>
 *
 * @param <K> The type of the Trevni key to read.
 * @param <V> The type of the Trevni value to read.
 *
 *            <p>
 *            A subset schema to be read may be specified with
 *            {@link org.apache.aingle.mapreduce.AIngleJob#setInputKeySchema} and
 *            {@link org.apache.aingle.mapreduce.AIngleJob#setInputValueSchema}.
 */
public class AIngleTrevniKeyValueInputFormat<K, V> extends FileInputFormat<AIngleKey<K>, AIngleValue<V>> {

  /** {@inheritDoc} */
  @Override
  public RecordReader<AIngleKey<K>, AIngleValue<V>> createRecordReader(InputSplit split, TaskAttemptContext context)
      throws IOException, InterruptedException {

    return new AIngleTrevniKeyValueRecordReader<>();
  }

}
