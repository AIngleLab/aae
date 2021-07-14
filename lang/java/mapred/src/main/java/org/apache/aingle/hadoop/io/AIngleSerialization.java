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

import java.lang.reflect.Constructor;
import java.util.Collection;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.aingle.mapred.AIngleWrapper;
import org.apache.aingle.reflect.ReflectData;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.Serialization;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.util.ReflectionUtils;

/**
 * The {@link org.apache.hadoop.io.serializer.Serialization} used by jobs
 * configured with {@link org.apache.aingle.mapreduce.AIngleJob}.
 *
 * @param <T> The Java type of the AIngle data to serialize.
 */
public class AIngleSerialization<T> extends Configured implements Serialization<AIngleWrapper<T>> {
  /**
   * Conf key for the writer schema of the AIngleKey datum being
   * serialized/deserialized.
   */
  private static final String CONF_KEY_WRITER_SCHEMA = "aingle.serialization.key.writer.schema";

  /**
   * Conf key for the reader schema of the AIngleKey datum being
   * serialized/deserialized.
   */
  private static final String CONF_KEY_READER_SCHEMA = "aingle.serialization.key.reader.schema";

  /**
   * Conf key for the writer schema of the AIngleValue datum being
   * serialized/deserialized.
   */
  private static final String CONF_VALUE_WRITER_SCHEMA = "aingle.serialization.value.writer.schema";

  /**
   * Conf key for the reader schema of the AIngleValue datum being
   * serialized/deserialized.
   */
  private static final String CONF_VALUE_READER_SCHEMA = "aingle.serialization.value.reader.schema";

  /** Conf key for the data model implementation class. */
  private static final String CONF_DATA_MODEL = "aingle.serialization.data.model";

  /** {@inheritDoc} */
  @Override
  public boolean accept(Class<?> c) {
    return AIngleKey.class.isAssignableFrom(c) || AIngleValue.class.isAssignableFrom(c);
  }

  /**
   * Gets an object capable of deserializing the output from a Mapper.
   *
   * @param c The class to get a deserializer for.
   * @return A deserializer for objects of class <code>c</code>.
   */
  @Override
  public Deserializer<AIngleWrapper<T>> getDeserializer(Class<AIngleWrapper<T>> c) {
    Configuration conf = getConf();
    GenericData dataModel = createDataModel(conf);
    if (AIngleKey.class.isAssignableFrom(c)) {
      Schema writerSchema = getKeyWriterSchema(conf);
      Schema readerSchema = getKeyReaderSchema(conf);
      DatumReader<T> datumReader = (readerSchema != null) ? dataModel.createDatumReader(writerSchema, readerSchema)
          : dataModel.createDatumReader(writerSchema);
      return new AIngleKeyDeserializer<>(writerSchema, readerSchema, datumReader);
    } else if (AIngleValue.class.isAssignableFrom(c)) {
      Schema writerSchema = getValueWriterSchema(conf);
      Schema readerSchema = getValueReaderSchema(conf);
      DatumReader<T> datumReader = (readerSchema != null) ? dataModel.createDatumReader(writerSchema, readerSchema)
          : dataModel.createDatumReader(writerSchema);
      return new AIngleValueDeserializer<>(writerSchema, readerSchema, datumReader);
    } else {
      throw new IllegalStateException("Only AIngleKey and AIngleValue are supported.");
    }
  }

  /**
   * Gets an object capable of serializing output from a Mapper.
   *
   * @param c The class to get a serializer for.
   * @return A serializer for objects of class <code>c</code>.
   */
  @Override
  public Serializer<AIngleWrapper<T>> getSerializer(Class<AIngleWrapper<T>> c) {
    Configuration conf = getConf();
    Schema schema;
    if (AIngleKey.class.isAssignableFrom(c)) {
      schema = getKeyWriterSchema(conf);
    } else if (AIngleValue.class.isAssignableFrom(c)) {
      schema = getValueWriterSchema(conf);
    } else {
      throw new IllegalStateException("Only AIngleKey and AIngleValue are supported.");
    }
    GenericData dataModel = createDataModel(conf);
    DatumWriter<T> datumWriter = dataModel.createDatumWriter(schema);
    return new AIngleSerializer<>(schema, datumWriter);
  }

  /**
   * Adds the AIngleSerialization scheme to the configuration, so
   * SerializationFactory instances constructed from the given configuration will
   * be aware of it.
   *
   * @param conf The configuration to add AIngleSerialization to.
   */
  public static void addToConfiguration(Configuration conf) {
    Collection<String> serializations = conf.getStringCollection("io.serializations");
    if (!serializations.contains(AIngleSerialization.class.getName())) {
      serializations.add(AIngleSerialization.class.getName());
      conf.setStrings("io.serializations", serializations.toArray(new String[0]));
    }
  }

  /**
   * Sets the writer schema of the AIngleKey datum that is being
   * serialized/deserialized.
   *
   * @param conf   The configuration.
   * @param schema The AIngle key schema.
   */
  public static void setKeyWriterSchema(Configuration conf, Schema schema) {
    if (null == schema) {
      throw new IllegalArgumentException("Writer schema may not be null");
    }
    conf.set(CONF_KEY_WRITER_SCHEMA, schema.toString());
  }

  /**
   * Sets the reader schema of the AIngleKey datum that is being
   * serialized/deserialized.
   *
   * @param conf   The configuration.
   * @param schema The AIngle key schema.
   */
  public static void setKeyReaderSchema(Configuration conf, Schema schema) {
    conf.set(CONF_KEY_READER_SCHEMA, schema.toString());
  }

  /**
   * Sets the writer schema of the AIngleValue datum that is being
   * serialized/deserialized.
   *
   * @param conf   The configuration.
   * @param schema The AIngle value schema.
   */
  public static void setValueWriterSchema(Configuration conf, Schema schema) {
    if (null == schema) {
      throw new IllegalArgumentException("Writer schema may not be null");
    }
    conf.set(CONF_VALUE_WRITER_SCHEMA, schema.toString());
  }

  /**
   * Sets the reader schema of the AIngleValue datum that is being
   * serialized/deserialized.
   *
   * @param conf   The configuration.
   * @param schema The AIngle value schema.
   */
  public static void setValueReaderSchema(Configuration conf, Schema schema) {
    conf.set(CONF_VALUE_READER_SCHEMA, schema.toString());
  }

  /**
   * Sets the data model class for de/serialization.
   *
   * @param conf       The configuration.
   * @param modelClass The data model class.
   */
  public static void setDataModelClass(Configuration conf, Class<? extends GenericData> modelClass) {
    conf.setClass(CONF_DATA_MODEL, modelClass, GenericData.class);
  }

  /**
   * Gets the writer schema of the AIngleKey datum that is being
   * serialized/deserialized.
   *
   * @param conf The configuration.
   * @return The AIngle key writer schema, or null if none was set.
   */
  public static Schema getKeyWriterSchema(Configuration conf) {
    String json = conf.get(CONF_KEY_WRITER_SCHEMA);
    return null == json ? null : new Schema.Parser().parse(json);
  }

  /**
   * Gets the reader schema of the AIngleKey datum that is being
   * serialized/deserialized.
   *
   * @param conf The configuration.
   * @return The AIngle key reader schema, or null if none was set.
   */
  public static Schema getKeyReaderSchema(Configuration conf) {
    String json = conf.get(CONF_KEY_READER_SCHEMA);
    return null == json ? null : new Schema.Parser().parse(json);
  }

  /**
   * Gets the writer schema of the AIngleValue datum that is being
   * serialized/deserialized.
   *
   * @param conf The configuration.
   * @return The AIngle value writer schema, or null if none was set.
   */
  public static Schema getValueWriterSchema(Configuration conf) {
    String json = conf.get(CONF_VALUE_WRITER_SCHEMA);
    return null == json ? null : new Schema.Parser().parse(json);
  }

  /**
   * Gets the reader schema of the AIngleValue datum that is being
   * serialized/deserialized.
   *
   * @param conf The configuration.
   * @return The AIngle value reader schema, or null if none was set.
   */
  public static Schema getValueReaderSchema(Configuration conf) {
    String json = conf.get(CONF_VALUE_READER_SCHEMA);
    return null == json ? null : new Schema.Parser().parse(json);
  }

  /**
   * Gets the data model class for de/serialization.
   *
   * @param conf The configuration.
   */
  public static Class<? extends GenericData> getDataModelClass(Configuration conf) {
    return conf.getClass(CONF_DATA_MODEL, ReflectData.class, GenericData.class);
  }

  private static GenericData newDataModelInstance(Class<? extends GenericData> modelClass, Configuration conf) {
    GenericData dataModel;
    try {
      Constructor<? extends GenericData> ctor = modelClass.getDeclaredConstructor(ClassLoader.class);
      ctor.setAccessible(true);
      dataModel = ctor.newInstance(conf.getClassLoader());
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
    ReflectionUtils.setConf(dataModel, conf);
    return dataModel;
  }

  /**
   * Gets an instance of data model implementation, defaulting to
   * {@link ReflectData} if not explicitly specified.
   *
   * @param conf The job configuration.
   * @return Instance of the job data model implementation.
   */
  public static GenericData createDataModel(Configuration conf) {
    Class<? extends GenericData> modelClass = getDataModelClass(conf);
    return newDataModelInstance(modelClass, conf);
  }
}
