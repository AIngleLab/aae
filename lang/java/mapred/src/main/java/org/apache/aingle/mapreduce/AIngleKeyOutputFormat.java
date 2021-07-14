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
import java.io.OutputStream;

import org.apache.aingle.Schema;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.hadoop.io.AIngleSerialization;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

/**
 * FileOutputFormat for writing AIngle container files.
 *
 * <p>
 * Since AIngle container files only contain records (not key/value pairs), this
 * output format ignores the value.
 * </p>
 *
 * @param <T> The (java) type of the AIngle data to write.
 */
public class AIngleKeyOutputFormat<T> extends AIngleOutputFormatBase<AIngleKey<T>, NullWritable> {
  /** A factory for creating record writers. */
  private final RecordWriterFactory mRecordWriterFactory;

  /**
   * Constructor.
   */
  public AIngleKeyOutputFormat() {
    this(new RecordWriterFactory());
  }

  /**
   * Constructor.
   *
   * @param recordWriterFactory A factory for creating record writers.
   */
  protected AIngleKeyOutputFormat(RecordWriterFactory recordWriterFactory) {
    mRecordWriterFactory = recordWriterFactory;
  }

  /**
   * A factory for creating record writers.
   *
   * @param <T> The java type of the aingle record to write.
   */
  protected static class RecordWriterFactory<T> {
    /**
     * Creates a new record writer instance.
     *
     * @param writerSchema     The writer schema for the records to write.
     * @param compressionCodec The compression type for the writer file.
     * @param outputStream     The target output stream for the records.
     * @param syncInterval     The sync interval for the writer file.
     */
    protected RecordWriter<AIngleKey<T>, NullWritable> create(Schema writerSchema, GenericData dataModel,
        CodecFactory compressionCodec, OutputStream outputStream, int syncInterval) throws IOException {
      return new AIngleKeyRecordWriter<>(writerSchema, dataModel, compressionCodec, outputStream, syncInterval);
    }
  }

  /** {@inheritDoc} */
  @Override
  @SuppressWarnings("unchecked")
  public RecordWriter<AIngleKey<T>, NullWritable> getRecordWriter(TaskAttemptContext context) throws IOException {
    Configuration conf = context.getConfiguration();
    // Get the writer schema.
    Schema writerSchema = AIngleJob.getOutputKeySchema(conf);
    boolean isMapOnly = context.getNumReduceTasks() == 0;
    if (isMapOnly) {
      Schema mapOutputSchema = AIngleJob.getMapOutputKeySchema(conf);
      if (mapOutputSchema != null) {
        writerSchema = mapOutputSchema;
      }
    }
    if (null == writerSchema) {
      throw new IOException("AIngleKeyOutputFormat requires an output schema. Use AIngleJob.setOutputKeySchema().");
    }

    GenericData dataModel = AIngleSerialization.createDataModel(conf);

    OutputStream out = getAIngleFileOutputStream(context);
    try {
      return mRecordWriterFactory.create(writerSchema, dataModel, getCompressionCodec(context), out,
          getSyncInterval(context));
    } catch (IOException e) {
      out.close();
      throw e;
    }
  }
}
