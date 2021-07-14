/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.mapred.AIngleKey;
import org.apache.hadoop.io.NullWritable;

/**
 * Reads records from an input split representing a chunk of an Trenvi container
 * file.
 *
 * @param <T> The (java) type of data in Trevni container file.
 */
public class AIngleTrevniKeyRecordReader<T> extends AIngleTrevniRecordReaderBase<AIngleKey<T>, NullWritable, T> {

  /** A reusable object to hold records of the AIngle container file. */
  private final AIngleKey<T> mCurrentKey = new AIngleKey<>();

  /** {@inheritDoc} */
  @Override
  public AIngleKey<T> getCurrentKey() throws IOException, InterruptedException {
    return mCurrentKey;
  }

  /** {@inheritDoc} */
  @Override
  public NullWritable getCurrentValue() throws IOException, InterruptedException {
    return NullWritable.get();
  }

  /** {@inheritDoc} */
  @Override
  public boolean nextKeyValue() throws IOException, InterruptedException {
    boolean hasNext = super.nextKeyValue();
    mCurrentKey.datum(getCurrentRecord());
    return hasNext;
  }

}
