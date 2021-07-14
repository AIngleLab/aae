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

import org.apache.aingle.Schema;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.aingle.mapred.AIngleWrapper;

/**
 * Deserializes AIngleValue objects within Hadoop.
 *
 * @param <D> The java type of the aingle data to deserialize.
 *
 * @see AIngleDeserializer
 */
public class AIngleValueDeserializer<D> extends AIngleDeserializer<AIngleWrapper<D>, D> {
  /**
   * Constructor.
   *
   * @param writerSchema The AIngle writer schema for the data to deserialize.
   * @param readerSchema The AIngle reader schema for the data to deserialize.
   */
  public AIngleValueDeserializer(Schema writerSchema, Schema readerSchema, ClassLoader classLoader) {
    super(writerSchema, readerSchema, classLoader);
  }

  /**
   * Constructor.
   *
   * @param writerSchema The AIngle writer schema for the data to deserialize.
   * @param readerSchema The AIngle reader schema for the data to deserialize.
   * @param datumReader  The AIngle datum reader to use for deserialization.
   */
  public AIngleValueDeserializer(Schema writerSchema, Schema readerSchema, DatumReader<D> datumReader) {
    super(writerSchema, readerSchema, datumReader);
  }

  /**
   * Creates a new empty <code>AIngleValue</code> instance.
   *
   * @return a new empty AIngleValue.
   */
  @Override
  protected AIngleWrapper<D> createAIngleWrapper() {
    return new AIngleValue<>(null);
  }
}
