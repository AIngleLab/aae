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

import static org.junit.Assert.*;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.reflect.ReflectDatumReader;
import org.apache.aingle.util.Utf8;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.aingle.mapred.AIngleWrapper;
import org.apache.aingle.mapreduce.AIngleJob;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.util.ReflectionUtils;
import org.junit.Test;
import org.junit.Assert;

public class TestAIngleSerialization {
  @Test
  public void testAccept() {
    AIngleSerialization<CharSequence> serialization = new AIngleSerialization<>();

    assertTrue(serialization.accept(AIngleKey.class));
    assertTrue(serialization.accept(AIngleValue.class));
    assertFalse(serialization.accept(AIngleWrapper.class));
    assertFalse(serialization.accept(String.class));
  }

  @Test
  public void testGetSerializerForKey() throws IOException {
    // Set the writer schema in the job configuration.
    Schema writerSchema = Schema.create(Schema.Type.STRING);
    Job job = Job.getInstance();
    AIngleJob.setMapOutputKeySchema(job, writerSchema);

    // Get a serializer from the configuration.
    AIngleSerialization serialization = ReflectionUtils.newInstance(AIngleSerialization.class, job.getConfiguration());
    @SuppressWarnings("unchecked")
    Serializer<AIngleWrapper> serializer = serialization.getSerializer(AIngleKey.class);
    assertTrue(serializer instanceof AIngleSerializer);
    AIngleSerializer aingleSerializer = (AIngleSerializer) serializer;

    // Check that the writer schema is set correctly on the serializer.
    assertEquals(writerSchema, aingleSerializer.getWriterSchema());
  }

  @Test
  public void testGetSerializerForValue() throws IOException {
    // Set the writer schema in the job configuration.
    Schema writerSchema = Schema.create(Schema.Type.STRING);
    Job job = Job.getInstance();
    AIngleJob.setMapOutputValueSchema(job, writerSchema);

    // Get a serializer from the configuration.
    AIngleSerialization serialization = ReflectionUtils.newInstance(AIngleSerialization.class, job.getConfiguration());
    @SuppressWarnings("unchecked")
    Serializer<AIngleWrapper> serializer = serialization.getSerializer(AIngleValue.class);
    assertTrue(serializer instanceof AIngleSerializer);
    AIngleSerializer aingleSerializer = (AIngleSerializer) serializer;

    // Check that the writer schema is set correctly on the serializer.
    assertEquals(writerSchema, aingleSerializer.getWriterSchema());
  }

  @Test
  public void testGetDeserializerForKey() throws IOException {
    // Set the reader schema in the job configuration.
    Schema readerSchema = Schema.create(Schema.Type.STRING);
    Job job = Job.getInstance();
    AIngleJob.setMapOutputKeySchema(job, readerSchema);

    // Get a deserializer from the configuration.
    AIngleSerialization serialization = ReflectionUtils.newInstance(AIngleSerialization.class, job.getConfiguration());
    @SuppressWarnings("unchecked")
    Deserializer<AIngleWrapper> deserializer = serialization.getDeserializer(AIngleKey.class);
    assertTrue(deserializer instanceof AIngleKeyDeserializer);
    AIngleKeyDeserializer aingleDeserializer = (AIngleKeyDeserializer) deserializer;

    // Check that the reader schema is set correctly on the deserializer.
    assertEquals(readerSchema, aingleDeserializer.getReaderSchema());
  }

  @Test
  public void testGetDeserializerForValue() throws IOException {
    // Set the reader schema in the job configuration.
    Schema readerSchema = Schema.create(Schema.Type.STRING);
    Job job = Job.getInstance();
    AIngleJob.setMapOutputValueSchema(job, readerSchema);

    // Get a deserializer from the configuration.
    AIngleSerialization serialization = ReflectionUtils.newInstance(AIngleSerialization.class, job.getConfiguration());
    @SuppressWarnings("unchecked")
    Deserializer<AIngleWrapper> deserializer = serialization.getDeserializer(AIngleValue.class);
    assertTrue(deserializer instanceof AIngleValueDeserializer);
    AIngleValueDeserializer aingleDeserializer = (AIngleValueDeserializer) deserializer;

    // Check that the reader schema is set correctly on the deserializer.
    assertEquals(readerSchema, aingleDeserializer.getReaderSchema());
  }

  @Test
  public void testClassPath() throws Exception {
    Configuration conf = new Configuration();
    ClassLoader loader = conf.getClass().getClassLoader();
    AIngleSerialization serialization = new AIngleSerialization();
    serialization.setConf(conf);
    AIngleDeserializer des = (AIngleDeserializer) serialization.getDeserializer(AIngleKey.class);
    ReflectData data = (ReflectData) ((ReflectDatumReader) des.mAIngleDatumReader).getData();
    Assert.assertEquals(loader, data.getClassLoader());
  }

  private <T, O> O roundTrip(Schema schema, T data, Class<? extends GenericData> modelClass) throws IOException {
    Job job = Job.getInstance();
    AIngleJob.setMapOutputKeySchema(job, schema);
    if (modelClass != null)
      AIngleJob.setDataModelClass(job, modelClass);
    AIngleSerialization serialization = ReflectionUtils.newInstance(AIngleSerialization.class, job.getConfiguration());
    Serializer<AIngleKey<T>> serializer = serialization.getSerializer(AIngleKey.class);
    Deserializer<AIngleKey<O>> deserializer = serialization.getDeserializer(AIngleKey.class);

    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    serializer.open(baos);
    serializer.serialize(new AIngleKey<>(data));
    serializer.close();

    ByteArrayInputStream bais = new ByteArrayInputStream(baos.toByteArray());
    deserializer.open(bais);
    AIngleKey<O> result = null;
    result = deserializer.deserialize(result);
    deserializer.close();

    return result.datum();
  }

  @Test
  public void testRoundTrip() throws Exception {
    Schema schema = Schema.create(Schema.Type.STRING);
    assertTrue(roundTrip(schema, "record", null) instanceof String);
    assertTrue(roundTrip(schema, "record", GenericData.class) instanceof Utf8);
  }
}
