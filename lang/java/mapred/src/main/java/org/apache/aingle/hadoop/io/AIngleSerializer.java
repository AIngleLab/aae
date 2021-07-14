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

package org.apache.aingle.hadoop.io;

import java.io.IOException;
import java.io.OutputStream;

import org.apache.aingle.Schema;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.mapred.AIngleWrapper;
import org.apache.aingle.reflect.ReflectDatumWriter;
import org.apache.hadoop.io.serializer.Serializer;

/**
 * Serializes AIngleWrapper objects within Hadoop.
 *
 * <p>
 * Keys and values containing AIngle types are more efficiently serialized outside
 * of the WritableSerialization model, so they are wrapped in
 * {@link org.apache.aingle.mapred.AIngleWrapper} objects and serialization is
 * handled by this class.
 * </p>
 *
 * <p>
 * MapReduce jobs that use AIngleWrapper objects as keys or values need to be
 * configured with {@link AIngleSerialization}. Use
 * {@link org.apache.aingle.mapreduce.AIngleJob} to help with Job configuration.
 * </p>
 *
 * @param <T> The Java type of the AIngle data.
 */
public class AIngleSerializer<T> implements Serializer<AIngleWrapper<T>> {

  /** An factory for creating AIngle datum encoders. */
  private static final EncoderFactory ENCODER_FACTORY = new EncoderFactory();

  /** The writer schema for the data to serialize. */
  private final Schema mWriterSchema;

  /** The AIngle datum writer for serializing. */
  private final DatumWriter<T> mAIngleDatumWriter;

  /** The AIngle encoder for serializing. */
  private BinaryEncoder mAIngleEncoder;

  /** The output stream for serializing. */
  private OutputStream mOutputStream;

  /**
   * Constructor.
   *
   * @param writerSchema The writer schema for the AIngle data being serialized.
   */
  public AIngleSerializer(Schema writerSchema) {
    if (null == writerSchema) {
      throw new IllegalArgumentException("Writer schema may not be null");
    }
    mWriterSchema = writerSchema;
    mAIngleDatumWriter = new ReflectDatumWriter<>(writerSchema);
  }

  /**
   * Constructor.
   *
   * @param writerSchema The writer schema for the AIngle data being serialized.
   * @param datumWriter  The datum writer to use for serialization.
   */
  public AIngleSerializer(Schema writerSchema, DatumWriter<T> datumWriter) {
    if (null == writerSchema) {
      throw new IllegalArgumentException("Writer schema may not be null");
    }
    mWriterSchema = writerSchema;
    mAIngleDatumWriter = datumWriter;
  }

  /**
   * Gets the writer schema being used for serialization.
   *
   * @return The writer schema.
   */
  public Schema getWriterSchema() {
    return mWriterSchema;
  }

  /** {@inheritDoc} */
  @Override
  public void open(OutputStream outputStream) throws IOException {
    mOutputStream = outputStream;
    mAIngleEncoder = ENCODER_FACTORY.binaryEncoder(outputStream, mAIngleEncoder);
  }

  /** {@inheritDoc} */
  @Override
  public void serialize(AIngleWrapper<T> aingleWrapper) throws IOException {
    mAIngleDatumWriter.write(aingleWrapper.datum(), mAIngleEncoder);
    // This would be a lot faster if the Serializer interface had a flush() method
    // and the
    // Hadoop framework called it when needed. For now, we'll have to flush on every
    // record.
    mAIngleEncoder.flush();
  }

  /** {@inheritDoc} */
  @Override
  public void close() throws IOException {
    mOutputStream.close();
  }
}
