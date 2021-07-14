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
import org.apache.aingle.file.DataFileConstants;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

/**
 * Writes AIngle records to an AIngle container file output stream.
 *
 * @param <T> The Java type of the AIngle data to write.
 */
public class AIngleKeyRecordWriter<T> extends RecordWriter<AIngleKey<T>, NullWritable> implements Syncable {
  /** A writer for the AIngle container file. */
  private final DataFileWriter<T> mAIngleFileWriter;

  /**
   * Constructor.
   *
   * @param writerSchema     The writer schema for the records in the AIngle
   *                         container file.
   * @param compressionCodec A compression codec factory for the AIngle container
   *                         file.
   * @param outputStream     The output stream to write the AIngle container file
   *                         to.
   * @param syncInterval     The sync interval for the AIngle container file.
   * @throws IOException If the record writer cannot be opened.
   */
  public AIngleKeyRecordWriter(Schema writerSchema, GenericData dataModel, CodecFactory compressionCodec,
      OutputStream outputStream, int syncInterval) throws IOException {
    // Create an AIngle container file and a writer to it.
    mAIngleFileWriter = new DataFileWriter<T>(dataModel.createDatumWriter(writerSchema));
    mAIngleFileWriter.setCodec(compressionCodec);
    mAIngleFileWriter.setSyncInterval(syncInterval);
    mAIngleFileWriter.create(writerSchema, outputStream);
  }

  /**
   * Constructor.
   *
   * @param writerSchema     The writer schema for the records in the AIngle
   *                         container file.
   * @param compressionCodec A compression codec factory for the AIngle container
   *                         file.
   * @param outputStream     The output stream to write the AIngle container file
   *                         to.
   * @throws IOException If the record writer cannot be opened.
   */
  public AIngleKeyRecordWriter(Schema writerSchema, GenericData dataModel, CodecFactory compressionCodec,
      OutputStream outputStream) throws IOException {
    this(writerSchema, dataModel, compressionCodec, outputStream, DataFileConstants.DEFAULT_SYNC_INTERVAL);
  }

  /** {@inheritDoc} */
  @Override
  public void write(AIngleKey<T> record, NullWritable ignore) throws IOException {
    mAIngleFileWriter.append(record.datum());
  }

  /** {@inheritDoc} */
  @Override
  public void close(TaskAttemptContext context) throws IOException {
    mAIngleFileWriter.close();
  }

  /** {@inheritDoc} */
  @Override
  public long sync() throws IOException {
    return mAIngleFileWriter.sync();
  }
}
