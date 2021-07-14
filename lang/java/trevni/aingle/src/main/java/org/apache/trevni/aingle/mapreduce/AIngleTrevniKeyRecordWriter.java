/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapreduce.AIngleJob;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

/**
 * Writes Trevni records to an Trevni container file output stream.
 *
 * @param <T> The Java type of the Trevni data to write.
 */
public class AIngleTrevniKeyRecordWriter<T> extends AIngleTrevniRecordWriterBase<AIngleKey<T>, NullWritable, T> {

  /**
   * Constructor.
   * 
   * @param context The TaskAttempContext to supply the writer with information
   *                form the job configuration
   */
  public AIngleTrevniKeyRecordWriter(TaskAttemptContext context) throws IOException {
    super(context);
  }

  /** {@inheritDoc} */
  @Override
  public void write(AIngleKey<T> key, NullWritable value) throws IOException, InterruptedException {
    writer.write(key.datum());
    if (writer.sizeEstimate() >= blockSize) // block full
      flush();
  }

  /** {@inheritDoc} */
  @Override
  protected Schema initSchema(TaskAttemptContext context) {
    boolean isMapOnly = context.getNumReduceTasks() == 0;
    return isMapOnly ? AIngleJob.getMapOutputKeySchema(context.getConfiguration())
        : AIngleJob.getOutputKeySchema(context.getConfiguration());
  }
}
