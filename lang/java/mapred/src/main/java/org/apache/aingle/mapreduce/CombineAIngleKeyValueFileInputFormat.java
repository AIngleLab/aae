/**

 */

package org.apache.aingle.mapreduce;

import java.io.IOException;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.CombineFileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.CombineFileRecordReader;
import org.apache.hadoop.mapreduce.lib.input.CombineFileRecordReaderWrapper;
import org.apache.hadoop.mapreduce.lib.input.CombineFileSplit;

/**
 * A combine aingle keyvalue file input format that can combine small aingle files
 * into mappers.
 *
 * @param <K> The type of the AIngle key to read.
 * @param <V> The type of the AIngle value to read.
 */
public class CombineAIngleKeyValueFileInputFormat<K, V> extends CombineFileInputFormat<AIngleKey<K>, AIngleValue<V>> {

  @Override
  public RecordReader<AIngleKey<K>, AIngleValue<V>> createRecordReader(InputSplit inputSplit,
      TaskAttemptContext taskAttemptContext) throws IOException {
    return new CombineFileRecordReader((CombineFileSplit) inputSplit, taskAttemptContext,
        CombineAIngleKeyValueFileInputFormat.AIngleKeyValueFileRecordReaderWrapper.class);
  }

  /**
   * A record reader that may be passed to <code>CombineFileRecordReader</code> so
   * that it can be used in a <code>CombineFileInputFormat</code>-equivalent for
   * <code>AIngleKeyValueInputFormat</code>.
   *
   * @see CombineFileRecordReader
   * @see CombineFileInputFormat
   * @see AIngleKeyValueInputFormat
   */
  private static class AIngleKeyValueFileRecordReaderWrapper<K, V>
      extends CombineFileRecordReaderWrapper<AIngleKey<K>, AIngleValue<V>> {
    // this constructor signature is required by CombineFileRecordReader
    public AIngleKeyValueFileRecordReaderWrapper(CombineFileSplit split, TaskAttemptContext context, Integer idx)
        throws IOException, InterruptedException {
      super(new AIngleKeyValueInputFormat<>(), split, context, idx);
    }
  }
}
