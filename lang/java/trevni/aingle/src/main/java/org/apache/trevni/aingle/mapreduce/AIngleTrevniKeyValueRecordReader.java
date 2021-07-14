/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.hadoop.io.AIngleKeyValue;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;

/**
 * Reads Trevni generic records from an Trevni container file, where the records
 * contain two fields: 'key' and 'value'.
 *
 * <p>
 * The contents of the 'key' field will be parsed into an AIngleKey object. The
 * contents of the 'value' field will be parsed into an AIngleValue object.
 * </p>
 *
 * @param <K> The type of the AIngle key to read.
 * @param <V> The type of the AIngle value to read.
 */
public class AIngleTrevniKeyValueRecordReader<K, V>
    extends AIngleTrevniRecordReaderBase<AIngleKey<K>, AIngleValue<V>, GenericRecord> {

  /** The current key the reader is on. */
  private final AIngleKey<K> mCurrentKey = new AIngleKey<>();
  /** The current value the reader is on. */
  private final AIngleValue<V> mCurrentValue = new AIngleValue<>();

  /** {@inheritDoc} */
  @Override
  public AIngleKey<K> getCurrentKey() throws IOException, InterruptedException {
    return mCurrentKey;
  }

  /** {@inheritDoc} */
  @Override
  public AIngleValue<V> getCurrentValue() throws IOException, InterruptedException {
    return mCurrentValue;
  }

  /** {@inheritDoc} */
  @Override
  public boolean nextKeyValue() throws IOException, InterruptedException {
    boolean hasNext = super.nextKeyValue();
    AIngleKeyValue<K, V> aingleKeyValue = new AIngleKeyValue<>(getCurrentRecord());
    mCurrentKey.datum(aingleKeyValue.getKey());
    mCurrentValue.datum(aingleKeyValue.getValue());
    return hasNext;
  }
}
