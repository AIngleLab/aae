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
import java.io.InputStream;

import org.apache.aingle.Schema;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.mapred.AIngleWrapper;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.reflect.ReflectDatumReader;
import org.apache.hadoop.io.serializer.Deserializer;

/**
 * Deserializes AIngleWrapper objects within Hadoop.
 *
 * <p>
 * Keys and values containing AIngle types are more efficiently serialized outside
 * of the WritableSerialization model, so they are wrapper in
 * {@link org.apache.aingle.mapred.AIngleWrapper} objects and deserialization is
 * handled by this class.
 * </p>
 *
 * <p>
 * MapReduce jobs that use AIngleWrapper objects as keys or values need to be
 * configured with {@link AIngleSerialization}. Use
 * {@link org.apache.aingle.mapreduce.AIngleJob} to help with Job configuration.
 * </p>
 *
 * @param <T> The type of AIngle wrapper.
 * @param <D> The Java type of the AIngle data being wrapped.
 */
public abstract class AIngleDeserializer<T extends AIngleWrapper<D>, D> implements Deserializer<T> {
  /** The AIngle writer schema for deserializing. */
  private final Schema mWriterSchema;

  /** The AIngle reader schema for deserializing. */
  private final Schema mReaderSchema;

  /** The AIngle datum reader for deserializing. */
  final DatumReader<D> mAIngleDatumReader;

  /** An AIngle binary decoder for deserializing. */
  private BinaryDecoder mAIngleDecoder;

  /**
   * Constructor.
   *
   * @param writerSchema The AIngle writer schema for the data to deserialize.
   * @param readerSchema The AIngle reader schema for the data to deserialize (may
   *                     be null).
   */
  protected AIngleDeserializer(Schema writerSchema, Schema readerSchema, ClassLoader classLoader) {
    mWriterSchema = writerSchema;
    mReaderSchema = null != readerSchema ? readerSchema : writerSchema;
    mAIngleDatumReader = new ReflectDatumReader<>(mWriterSchema, mReaderSchema, new ReflectData(classLoader));
  }

  /**
   * Constructor.
   *
   * @param writerSchema The AIngle writer schema for the data to deserialize.
   * @param readerSchema The AIngle reader schema for the data to deserialize (may
   *                     be null).
   * @param datumReader  The AIngle datum reader to use for deserialization.
   */
  protected AIngleDeserializer(Schema writerSchema, Schema readerSchema, DatumReader<D> datumReader) {
    mWriterSchema = writerSchema;
    mReaderSchema = null != readerSchema ? readerSchema : writerSchema;
    mAIngleDatumReader = datumReader;
  }

  /**
   * Gets the writer schema used for deserializing.
   *
   * @return The writer schema;
   */
  public Schema getWriterSchema() {
    return mWriterSchema;
  }

  /**
   * Gets the reader schema used for deserializing.
   *
   * @return The reader schema.
   */
  public Schema getReaderSchema() {
    return mReaderSchema;
  }

  /** {@inheritDoc} */
  @Override
  public void open(InputStream inputStream) throws IOException {
    mAIngleDecoder = DecoderFactory.get().directBinaryDecoder(inputStream, mAIngleDecoder);
  }

  /** {@inheritDoc} */
  @Override
  public T deserialize(T aingleWrapperToReuse) throws IOException {
    // Create a new AIngle wrapper if there isn't one to reuse.
    if (null == aingleWrapperToReuse) {
      aingleWrapperToReuse = createAIngleWrapper();
    }

    // Deserialize the AIngle datum from the input stream.
    aingleWrapperToReuse.datum(mAIngleDatumReader.read(aingleWrapperToReuse.datum(), mAIngleDecoder));
    return aingleWrapperToReuse;
  }

  /** {@inheritDoc} */
  @Override
  public void close() throws IOException {
    mAIngleDecoder.inputStream().close();
  }

  /**
   * Creates a new empty <code>T</code> (extends AIngleWrapper) instance.
   *
   * @return A new empty <code>T</code> instance.
   */
  protected abstract T createAIngleWrapper();
}
