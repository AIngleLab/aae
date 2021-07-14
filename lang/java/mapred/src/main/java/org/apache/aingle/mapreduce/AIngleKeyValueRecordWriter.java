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
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.hadoop.io.AIngleDatumConverter;
import org.apache.aingle.hadoop.io.AIngleKeyValue;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

/**
 * Writes key/value pairs to an AIngle container file.
 *
 * <p>
 * Each entry in the AIngle container file will be a generic record with two
 * fields, named 'key' and 'value'. The input types may be basic Writable
 * objects like Text or IntWritable, or they may be AIngleWrapper subclasses
 * (AIngleKey or AIngleValue). Writable objects will be converted to their
 * corresponding AIngle types when written to the generic record key/value pair.
 * </p>
 *
 * @param <K> The type of key to write.
 * @param <V> The type of value to write.
 */
public class AIngleKeyValueRecordWriter<K, V> extends RecordWriter<K, V> implements Syncable {
  /** A writer for the AIngle container file. */
  private final DataFileWriter<GenericRecord> mAIngleFileWriter;

  /**
   * The writer schema for the generic record entries of the AIngle container file.
   */
  private final Schema mKeyValuePairSchema;

  /** A reusable AIngle generic record for writing key/value pairs to the file. */
  private final AIngleKeyValue<Object, Object> mOutputRecord;

  /** A helper object that converts the input key to an AIngle datum. */
  private final AIngleDatumConverter<K, ?> mKeyConverter;

  /** A helper object that converts the input value to an AIngle datum. */
  private final AIngleDatumConverter<V, ?> mValueConverter;

  /**
   * Constructor.
   *
   * @param keyConverter     A key to AIngle datum converter.
   * @param valueConverter   A value to AIngle datum converter.
   * @param dataModel        The data model for key and value.
   * @param compressionCodec A compression codec factory for the AIngle container
   *                         file.
   * @param outputStream     The output stream to write the AIngle container file
   *                         to.
   * @param syncInterval     The sync interval for the AIngle container file.
   * @throws IOException If the record writer cannot be opened.
   */
  public AIngleKeyValueRecordWriter(AIngleDatumConverter<K, ?> keyConverter, AIngleDatumConverter<V, ?> valueConverter,
      GenericData dataModel, CodecFactory compressionCodec, OutputStream outputStream, int syncInterval)
      throws IOException {
    // Create the generic record schema for the key/value pair.
    mKeyValuePairSchema = AIngleKeyValue.getSchema(keyConverter.getWriterSchema(), valueConverter.getWriterSchema());

    // Create an AIngle container file and a writer to it.
    mAIngleFileWriter = new DataFileWriter<GenericRecord>(dataModel.createDatumWriter(mKeyValuePairSchema));
    mAIngleFileWriter.setCodec(compressionCodec);
    mAIngleFileWriter.setSyncInterval(syncInterval);
    mAIngleFileWriter.create(mKeyValuePairSchema, outputStream);

    // Keep a reference to the converters.
    mKeyConverter = keyConverter;
    mValueConverter = valueConverter;

    // Create a reusable output record.
    mOutputRecord = new AIngleKeyValue<>(new GenericData.Record(mKeyValuePairSchema));
  }

  /**
   * Constructor.
   *
   * @param keyConverter     A key to AIngle datum converter.
   * @param valueConverter   A value to AIngle datum converter.
   * @param dataModel        The data model for key and value.
   * @param compressionCodec A compression codec factory for the AIngle container
   *                         file.
   * @param outputStream     The output stream to write the AIngle container file
   *                         to.
   * @throws IOException If the record writer cannot be opened.
   */
  public AIngleKeyValueRecordWriter(AIngleDatumConverter<K, ?> keyConverter, AIngleDatumConverter<V, ?> valueConverter,
      GenericData dataModel, CodecFactory compressionCodec, OutputStream outputStream) throws IOException {
    this(keyConverter, valueConverter, dataModel, compressionCodec, outputStream,
        DataFileConstants.DEFAULT_SYNC_INTERVAL);
  }

  /**
   * Gets the writer schema for the key/value pair generic record.
   *
   * @return The writer schema used for entries of the AIngle container file.
   */
  public Schema getWriterSchema() {
    return mKeyValuePairSchema;
  }

  /** {@inheritDoc} */
  @Override
  public void write(K key, V value) throws IOException {
    mOutputRecord.setKey(mKeyConverter.convert(key));
    mOutputRecord.setValue(mValueConverter.convert(value));
    mAIngleFileWriter.append(mOutputRecord.get());
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
