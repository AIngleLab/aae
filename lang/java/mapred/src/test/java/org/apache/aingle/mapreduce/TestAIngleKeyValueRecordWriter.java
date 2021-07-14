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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.hadoop.io.AIngleDatumConverter;
import org.apache.aingle.hadoop.io.AIngleDatumConverterFactory;
import org.apache.aingle.hadoop.io.AIngleKeyValue;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.aingle.mapred.FsInput;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.reflect.ReflectDatumReader;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.junit.Test;

public class TestAIngleKeyValueRecordWriter {
  @Test
  public void testWriteRecords() throws IOException {
    Job job = Job.getInstance();
    AIngleJob.setOutputValueSchema(job, TextStats.SCHEMA$);
    TaskAttemptContext context = createMock(TaskAttemptContext.class);

    replay(context);

    AIngleDatumConverterFactory factory = new AIngleDatumConverterFactory(job.getConfiguration());
    AIngleDatumConverter<Text, ?> keyConverter = factory.create(Text.class);
    AIngleValue<TextStats> aingleValue = new AIngleValue<>(null);
    @SuppressWarnings("unchecked")
    AIngleDatumConverter<AIngleValue<TextStats>, ?> valueConverter = factory
        .create((Class<AIngleValue<TextStats>>) aingleValue.getClass());
    CodecFactory compressionCodec = CodecFactory.nullCodec();
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

    // Use a writer to generate a AIngle container file in memory.
    // Write two records: <'apple', TextStats('apple')> and <'banana',
    // TextStats('banana')>.
    AIngleKeyValueRecordWriter<Text, AIngleValue<TextStats>> writer = new AIngleKeyValueRecordWriter<>(keyConverter,
        valueConverter, new ReflectData(), compressionCodec, outputStream);
    TextStats appleStats = new TextStats();
    appleStats.setName("apple");
    writer.write(new Text("apple"), new AIngleValue<>(appleStats));
    TextStats bananaStats = new TextStats();
    bananaStats.setName("banana");
    writer.write(new Text("banana"), new AIngleValue<>(bananaStats));
    writer.close(context);

    verify(context);

    ByteArrayInputStream inputStream = new ByteArrayInputStream(outputStream.toByteArray());
    Schema readerSchema = AIngleKeyValue.getSchema(Schema.create(Schema.Type.STRING), TextStats.SCHEMA$);
    DatumReader<GenericRecord> datumReader = new SpecificDatumReader<>(readerSchema);
    DataFileStream<GenericRecord> aingleFileReader = new DataFileStream<>(inputStream, datumReader);

    // Verify that the first record was written.
    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<CharSequence, TextStats> firstRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(firstRecord.get());
    assertEquals("apple", firstRecord.getKey().toString());
    assertEquals("apple", firstRecord.getValue().getName().toString());

    // Verify that the second record was written;
    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<CharSequence, TextStats> secondRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(secondRecord.get());
    assertEquals("banana", secondRecord.getKey().toString());
    assertEquals("banana", secondRecord.getValue().getName().toString());

    // That's all, folks.
    assertFalse(aingleFileReader.hasNext());
    aingleFileReader.close();
  }

  public static class R1 {
    String attribute;
  }

  @Test
  public void testUsingReflection() throws Exception {
    Job job = Job.getInstance();
    Schema schema = ReflectData.get().getSchema(R1.class);
    AIngleJob.setOutputValueSchema(job, schema);
    TaskAttemptContext context = createMock(TaskAttemptContext.class);
    replay(context);

    R1 record = new R1();
    record.attribute = "test";
    AIngleValue<R1> aingleValue = new AIngleValue<>(record);

    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    AIngleDatumConverterFactory factory = new AIngleDatumConverterFactory(job.getConfiguration());

    AIngleDatumConverter<Text, ?> keyConverter = factory.create(Text.class);

    @SuppressWarnings("unchecked")
    AIngleDatumConverter<AIngleValue<R1>, R1> valueConverter = factory.create((Class<AIngleValue<R1>>) aingleValue.getClass());

    AIngleKeyValueRecordWriter<Text, AIngleValue<R1>> writer = new AIngleKeyValueRecordWriter<>(keyConverter, valueConverter,
        new ReflectData(), CodecFactory.nullCodec(), outputStream);

    writer.write(new Text("reflectionData"), aingleValue);
    writer.close(context);

    verify(context);

    ByteArrayInputStream inputStream = new ByteArrayInputStream(outputStream.toByteArray());
    Schema readerSchema = AIngleKeyValue.getSchema(Schema.create(Schema.Type.STRING), schema);
    DatumReader<GenericRecord> datumReader = new ReflectDatumReader<>(readerSchema);
    DataFileStream<GenericRecord> aingleFileReader = new DataFileStream<>(inputStream, datumReader);

    // Verify that the first record was written.
    assertTrue(aingleFileReader.hasNext());

    // Verify that the record holds the same data that we've written
    AIngleKeyValue<CharSequence, R1> firstRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(firstRecord.get());
    assertEquals("reflectionData", firstRecord.getKey().toString());
    assertEquals(record.attribute, firstRecord.getValue().attribute);
    aingleFileReader.close();
  }

  @Test
  public void testSyncableWriteRecords() throws IOException {
    Job job = Job.getInstance();
    AIngleJob.setOutputValueSchema(job, TextStats.SCHEMA$);
    TaskAttemptContext context = createMock(TaskAttemptContext.class);

    replay(context);

    AIngleDatumConverterFactory factory = new AIngleDatumConverterFactory(job.getConfiguration());
    AIngleDatumConverter<Text, ?> keyConverter = factory.create(Text.class);
    AIngleValue<TextStats> aingleValue = new AIngleValue<>(null);
    @SuppressWarnings("unchecked")
    AIngleDatumConverter<AIngleValue<TextStats>, ?> valueConverter = factory
        .create((Class<AIngleValue<TextStats>>) aingleValue.getClass());
    CodecFactory compressionCodec = CodecFactory.nullCodec();
    FileOutputStream outputStream = new FileOutputStream(new File("target/temp.aingle"));

    // Write a marker followed by each record: <'apple', TextStats('apple')> and
    // <'banana', TextStats('banana')>.
    AIngleKeyValueRecordWriter<Text, AIngleValue<TextStats>> writer = new AIngleKeyValueRecordWriter<>(keyConverter,
        valueConverter, new ReflectData(), compressionCodec, outputStream);
    TextStats appleStats = new TextStats();
    appleStats.setName("apple");
    long pointOne = writer.sync();
    writer.write(new Text("apple"), new AIngleValue<>(appleStats));
    TextStats bananaStats = new TextStats();
    bananaStats.setName("banana");
    long pointTwo = writer.sync();
    writer.write(new Text("banana"), new AIngleValue<>(bananaStats));
    writer.close(context);

    verify(context);

    Configuration conf = new Configuration();
    conf.set("fs.default.name", "file:///");
    Path aingleFile = new Path("target/temp.aingle");
    DataFileReader<GenericData.Record> aingleFileReader = new DataFileReader<>(new FsInput(aingleFile, conf),
        new SpecificDatumReader<>());

    aingleFileReader.seek(pointTwo);
    // Verify that the second record was written;
    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<CharSequence, TextStats> secondRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(secondRecord.get());
    assertEquals("banana", secondRecord.getKey().toString());
    assertEquals("banana", secondRecord.getValue().getName().toString());

    aingleFileReader.seek(pointOne);
    // Verify that the first record was written.
    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<CharSequence, TextStats> firstRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(firstRecord.get());
    assertEquals("apple", firstRecord.getKey().toString());
    assertEquals("apple", firstRecord.getValue().getName().toString());

    // That's all, folks.
    aingleFileReader.close();
  }
}
