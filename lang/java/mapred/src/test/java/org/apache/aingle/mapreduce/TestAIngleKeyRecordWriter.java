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
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import org.apache.aingle.Schema;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.FsInput;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.junit.Test;

public class TestAIngleKeyRecordWriter {
  @Test
  public void testWrite() throws IOException {
    Schema writerSchema = Schema.create(Schema.Type.INT);
    GenericData dataModel = new ReflectData();
    CodecFactory compressionCodec = CodecFactory.nullCodec();
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    TaskAttemptContext context = createMock(TaskAttemptContext.class);

    replay(context);

    // Write an aingle container file with two records: 1 and 2.
    AIngleKeyRecordWriter<Integer> recordWriter = new AIngleKeyRecordWriter<>(writerSchema, dataModel, compressionCodec,
        outputStream);
    recordWriter.write(new AIngleKey<>(1), NullWritable.get());
    recordWriter.write(new AIngleKey<>(2), NullWritable.get());
    recordWriter.close(context);

    verify(context);

    // Verify that the file was written as expected.
    InputStream inputStream = new ByteArrayInputStream(outputStream.toByteArray());
    Schema readerSchema = Schema.create(Schema.Type.INT);
    DatumReader<Integer> datumReader = new SpecificDatumReader<>(readerSchema);
    DataFileStream<Integer> dataFileReader = new DataFileStream<>(inputStream, datumReader);

    assertTrue(dataFileReader.hasNext()); // Record 1.
    assertEquals(1, dataFileReader.next().intValue());
    assertTrue(dataFileReader.hasNext()); // Record 2.
    assertEquals(2, dataFileReader.next().intValue());
    assertFalse(dataFileReader.hasNext()); // No more records.

    dataFileReader.close();
  }

  @Test
  public void testSycnableWrite() throws IOException {
    Schema writerSchema = Schema.create(Schema.Type.INT);
    GenericData dataModel = new ReflectData();
    CodecFactory compressionCodec = CodecFactory.nullCodec();
    FileOutputStream outputStream = new FileOutputStream(new File("target/temp.aingle"));
    TaskAttemptContext context = createMock(TaskAttemptContext.class);

    replay(context);

    // Write an aingle container file with two records: 1 and 2.
    AIngleKeyRecordWriter<Integer> recordWriter = new AIngleKeyRecordWriter<>(writerSchema, dataModel, compressionCodec,
        outputStream);
    long positionOne = recordWriter.sync();
    recordWriter.write(new AIngleKey<>(1), NullWritable.get());
    long positionTwo = recordWriter.sync();
    recordWriter.write(new AIngleKey<>(2), NullWritable.get());
    recordWriter.close(context);

    verify(context);

    // Verify that the file was written as expected.
    Configuration conf = new Configuration();
    conf.set("fs.default.name", "file:///");
    Path aingleFile = new Path("target/temp.aingle");
    DataFileReader<GenericData.Record> dataFileReader = new DataFileReader<>(new FsInput(aingleFile, conf),
        new SpecificDatumReader<>());

    dataFileReader.seek(positionTwo);
    assertTrue(dataFileReader.hasNext()); // Record 2.
    assertEquals(2, dataFileReader.next());

    dataFileReader.seek(positionOne);
    assertTrue(dataFileReader.hasNext()); // Record 1.
    assertEquals(1, dataFileReader.next());

    dataFileReader.close();
  }
}
