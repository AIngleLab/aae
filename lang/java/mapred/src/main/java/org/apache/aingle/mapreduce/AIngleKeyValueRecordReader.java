/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

package org.apache.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.hadoop.io.AIngleKeyValue;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;

/**
 * Reads AIngle generic records from an AIngle container file, where the records
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
public class AIngleKeyValueRecordReader<K, V> extends AIngleRecordReaderBase<AIngleKey<K>, AIngleValue<V>, GenericRecord> {
  /** The current key the reader is on. */
  private final AIngleKey<K> mCurrentKey;

  /** The current value the reader is on. */
  private final AIngleValue<V> mCurrentValue;

  /**
   * Constructor.
   *
   * @param keyReaderSchema   The reader schema for the key within the generic
   *                          record.
   * @param valueReaderSchema The reader schema for the value within the generic
   *                          record.
   */
  public AIngleKeyValueRecordReader(Schema keyReaderSchema, Schema valueReaderSchema) {
    super(AIngleKeyValue.getSchema(keyReaderSchema, valueReaderSchema));
    mCurrentKey = new AIngleKey<>(null);
    mCurrentValue = new AIngleValue<>(null);
  }

  /** {@inheritDoc} */
  @Override
  public boolean nextKeyValue() throws IOException, InterruptedException {
    boolean hasNext = super.nextKeyValue();
    if (hasNext) {
      AIngleKeyValue<K, V> aingleKeyValue = new AIngleKeyValue<>(getCurrentRecord());
      mCurrentKey.datum(aingleKeyValue.getKey());
      mCurrentValue.datum(aingleKeyValue.getValue());
    } else {
      mCurrentKey.datum(null);
      mCurrentValue.datum(null);
    }
    return hasNext;
  }

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
}
