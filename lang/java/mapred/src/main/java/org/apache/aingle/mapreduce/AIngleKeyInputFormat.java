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
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A MapReduce InputFormat that can handle AIngle container files.
 *
 * <p>
 * Keys are AIngleKey wrapper objects that contain the AIngle data. Since AIngle
 * container files store only records (not key/value pairs), the value from this
 * InputFormat is a NullWritable.
 * </p>
 */
public class AIngleKeyInputFormat<T> extends FileInputFormat<AIngleKey<T>, NullWritable> {
  private static final Logger LOG = LoggerFactory.getLogger(AIngleKeyInputFormat.class);

  /** {@inheritDoc} */
  @Override
  public RecordReader<AIngleKey<T>, NullWritable> createRecordReader(InputSplit split, TaskAttemptContext context)
      throws IOException, InterruptedException {
    Schema readerSchema = AIngleJob.getInputKeySchema(context.getConfiguration());
    if (null == readerSchema) {
      LOG.warn("Reader schema was not set. Use AIngleJob.setInputKeySchema() if desired.");
      LOG.info("Using a reader schema equal to the writer schema.");
    }
    return new AIngleKeyRecordReader<>(readerSchema);
  }
}
