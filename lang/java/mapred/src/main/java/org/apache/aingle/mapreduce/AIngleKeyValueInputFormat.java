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
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A MapReduce InputFormat that reads from AIngle container files of key/value
 * generic records.
 *
 * <p>
 * AIngle container files that container generic records with the two fields 'key'
 * and 'value' are expected. The contents of the 'key' field will be used as the
 * job input key, and the contents of the 'value' field will be used as the job
 * output value.
 * </p>
 *
 * @param <K> The type of the AIngle key to read.
 * @param <V> The type of the AIngle value to read.
 */
public class AIngleKeyValueInputFormat<K, V> extends FileInputFormat<AIngleKey<K>, AIngleValue<V>> {
  private static final Logger LOG = LoggerFactory.getLogger(AIngleKeyValueInputFormat.class);

  /** {@inheritDoc} */
  @Override
  public RecordReader<AIngleKey<K>, AIngleValue<V>> createRecordReader(InputSplit split, TaskAttemptContext context)
      throws IOException, InterruptedException {
    Schema keyReaderSchema = AIngleJob.getInputKeySchema(context.getConfiguration());
    if (null == keyReaderSchema) {
      LOG.warn("Key reader schema was not set. Use AIngleJob.setInputKeySchema() if desired.");
      LOG.info("Using a key reader schema equal to the writer schema.");
    }
    Schema valueReaderSchema = AIngleJob.getInputValueSchema(context.getConfiguration());
    if (null == valueReaderSchema) {
      LOG.warn("Value reader schema was not set. Use AIngleJob.setInputValueSchema() if desired.");
      LOG.info("Using a value reader schema equal to the writer schema.");
    }
    return new AIngleKeyValueRecordReader<>(keyReaderSchema, valueReaderSchema);
  }
}
