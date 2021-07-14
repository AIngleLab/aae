/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.mapreduce.AIngleJob;
import org.apache.aingle.reflect.ReflectData;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.apache.trevni.aingle.AIngleColumnReader;
import org.apache.trevni.aingle.HadoopInput;

/**
 * Abstract base class for <code>RecordReader</code>s that read Trevni container
 * files.
 *
 * @param <K> The type of key the record reader should generate.
 * @param <V> The type of value the record reader should generate.
 * @param <T> The type of the entries within the Trevni container file being
 *            read.
 */
public abstract class AIngleTrevniRecordReaderBase<K, V, T> extends RecordReader<K, V> {

  /** The Trevni file reader */
  private AIngleColumnReader<T> reader;

  /** Number of rows in the Trevni file */
  private float rows;

  /** The current row number being read in */
  private long row;

  /** A reusable object to hold records of the AIngle container file. */
  private T mCurrentRecord;

  /** {@inheritDoc} */
  @Override
  public void initialize(InputSplit inputSplit, TaskAttemptContext context) throws IOException, InterruptedException {
    final FileSplit file = (FileSplit) inputSplit;
    context.setStatus(file.toString());

    final AIngleColumnReader.Params params = new AIngleColumnReader.Params(
        new HadoopInput(file.getPath(), context.getConfiguration()));
    params.setModel(ReflectData.get());

    if (AIngleJob.getInputKeySchema(context.getConfiguration()) != null) {
      params.setSchema(AIngleJob.getInputKeySchema(context.getConfiguration()));
    }

    reader = new AIngleColumnReader<>(params);
    rows = reader.getRowCount();
  }

  /** {@inheritDoc} */
  @Override
  public boolean nextKeyValue() throws IOException, InterruptedException {
    if (!reader.hasNext())
      return false;
    mCurrentRecord = reader.next();
    row++;
    return true;
  }

  /**
   * Gets the current record read from the Trevni container file.
   *
   * <p>
   * Calling <code>nextKeyValue()</code> moves this to the next record.
   * </p>
   *
   * @return The current Trevni record (may be null if no record has been read).
   */
  protected T getCurrentRecord() {
    return mCurrentRecord;
  }

  /** {@inheritDoc} */
  @Override
  public void close() throws IOException {
    reader.close();
  }

  /** {@inheritDoc} */
  @Override
  public float getProgress() throws IOException, InterruptedException {
    return row / rows;
  }
}
