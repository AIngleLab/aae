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

import org.apache.aingle.generic.GenericData;
import org.apache.aingle.hadoop.io.AIngleDatumConverter;
import org.apache.aingle.hadoop.io.AIngleDatumConverterFactory;
import org.apache.aingle.hadoop.io.AIngleSerialization;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

/**
 * FileOutputFormat for writing AIngle container files of key/value pairs.
 *
 * <p>
 * Since AIngle container files can only contain records (not key/value pairs),
 * this output format puts the key and value into an AIngle generic record with
 * two fields, named 'key' and 'value'.
 * </p>
 *
 * <p>
 * The keys and values given to this output format may be AIngle objects wrapped
 * in <code>AIngleKey</code> or <code>AIngleValue</code> objects. The basic Writable
 * types are also supported (e.g., IntWritable, Text); they will be converted to
 * their corresponding AIngle types.
 * </p>
 *
 * @param <K> The type of key. If an AIngle type, it must be wrapped in an
 *            <code>AIngleKey</code>.
 * @param <V> The type of value. If an AIngle type, it must be wrapped in an
 *            <code>AIngleValue</code>.
 */
public class AIngleKeyValueOutputFormat<K, V> extends AIngleOutputFormatBase<K, V> {
  /** {@inheritDoc} */
  @Override
  @SuppressWarnings("unchecked")
  public RecordWriter<K, V> getRecordWriter(TaskAttemptContext context) throws IOException {
    Configuration conf = context.getConfiguration();

    AIngleDatumConverterFactory converterFactory = new AIngleDatumConverterFactory(conf);

    AIngleDatumConverter<K, ?> keyConverter = converterFactory.create((Class<K>) context.getOutputKeyClass());
    AIngleDatumConverter<V, ?> valueConverter = converterFactory.create((Class<V>) context.getOutputValueClass());

    GenericData dataModel = AIngleSerialization.createDataModel(conf);

    OutputStream out = getAIngleFileOutputStream(context);
    try {
      return new AIngleKeyValueRecordWriter<>(keyConverter, valueConverter, dataModel, getCompressionCodec(context), out,
          getSyncInterval(context));
    } catch (IOException e) {
      out.close();
      throw e;
    }
  }
}
