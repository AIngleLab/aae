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
import org.apache.aingle.mapred.AIngleKey;
import org.apache.hadoop.io.NullWritable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Reads records from an input split representing a chunk of an AIngle container
 * file.
 *
 * @param <T> The (java) type of data in AIngle container file.
 */
public class AIngleKeyRecordReader<T> extends AIngleRecordReaderBase<AIngleKey<T>, NullWritable, T> {
  private static final Logger LOG = LoggerFactory.getLogger(AIngleKeyRecordReader.class);

  /** A reusable object to hold records of the AIngle container file. */
  private final AIngleKey<T> mCurrentRecord;

  /**
   * Constructor.
   *
   * @param readerSchema The reader schema to use for the records in the AIngle
   *                     container file.
   */
  public AIngleKeyRecordReader(Schema readerSchema) {
    super(readerSchema);
    mCurrentRecord = new AIngleKey<>(null);
  }

  /** {@inheritDoc} */
  @Override
  public boolean nextKeyValue() throws IOException, InterruptedException {
    boolean hasNext = super.nextKeyValue();
    mCurrentRecord.datum(getCurrentRecord());
    return hasNext;
  }

  /** {@inheritDoc} */
  @Override
  public AIngleKey<T> getCurrentKey() throws IOException, InterruptedException {
    return mCurrentRecord;
  }

  /** {@inheritDoc} */
  @Override
  public NullWritable getCurrentValue() throws IOException, InterruptedException {
    return NullWritable.get();
  }
}
