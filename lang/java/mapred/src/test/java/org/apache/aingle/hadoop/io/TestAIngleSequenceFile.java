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

import java.io.File;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestAIngleSequenceFile {
  // Disable checkstyle for this variable. It must be public to work with JUnit
  // @Rule.
  // CHECKSTYLE:OFF
  @Rule
  public TemporaryFolder mTempDir = new TemporaryFolder();
  // CHECKSTYLE:ON

  /** Tests that reading and writing aingle data works. */
  @Test
  @SuppressWarnings("unchecked")
  public void testReadAIngle() throws IOException {
    Path sequenceFilePath = new Path(new File(mTempDir.getRoot(), "output.seq").getPath());

    writeSequenceFile(sequenceFilePath, AIngleKey.class, AIngleValue.class, Schema.create(Schema.Type.STRING),
        Schema.create(Schema.Type.INT), new AIngleKey<CharSequence>("one"), new AIngleValue<>(1),
        new AIngleKey<CharSequence>("two"), new AIngleValue<>(2));

    Configuration conf = new Configuration();
    FileSystem fs = FileSystem.get(conf);
    AIngleSequenceFile.Reader.Options options = new AIngleSequenceFile.Reader.Options().withFileSystem(fs)
        .withInputPath(sequenceFilePath).withKeySchema(Schema.create(Schema.Type.STRING))
        .withValueSchema(Schema.create(Schema.Type.INT)).withConfiguration(conf);
    try (SequenceFile.Reader reader = new AIngleSequenceFile.Reader(options)) {
      AIngleKey<CharSequence> key = new AIngleKey<>();
      AIngleValue<Integer> value = new AIngleValue<>();

      // Read the first record.
      key = (AIngleKey<CharSequence>) reader.next(key);
      assertNotNull(key);
      assertEquals("one", key.datum().toString());
      value = (AIngleValue<Integer>) reader.getCurrentValue(value);
      assertNotNull(value);
      assertEquals(1, value.datum().intValue());

      // Read the second record.
      key = (AIngleKey<CharSequence>) reader.next(key);
      assertNotNull(key);
      assertEquals("two", key.datum().toString());
      value = (AIngleValue<Integer>) reader.getCurrentValue(value);
      assertNotNull(value);
      assertEquals(2, value.datum().intValue());

      assertNull("Should be no more records.", reader.next(key));
    }
  }

  /**
   * Tests that reading and writing aingle records without a reader schema works.
   */
  @Test
  @SuppressWarnings("unchecked")
  public void testReadAIngleWithoutReaderSchemas() throws IOException {
    Path sequenceFilePath = new Path(new File(mTempDir.getRoot(), "output.seq").getPath());

    writeSequenceFile(sequenceFilePath, AIngleKey.class, AIngleValue.class, Schema.create(Schema.Type.STRING),
        Schema.create(Schema.Type.INT), new AIngleKey<CharSequence>("one"), new AIngleValue<>(1),
        new AIngleKey<CharSequence>("two"), new AIngleValue<>(2));

    Configuration conf = new Configuration();
    FileSystem fs = FileSystem.get(conf);
    AIngleSequenceFile.Reader.Options options = new AIngleSequenceFile.Reader.Options().withFileSystem(fs)
        .withInputPath(sequenceFilePath).withConfiguration(conf);

    try (SequenceFile.Reader reader = new AIngleSequenceFile.Reader(options)) {
      AIngleKey<CharSequence> key = new AIngleKey<>();
      AIngleValue<Integer> value = new AIngleValue<>();

      // Read the first record.
      key = (AIngleKey<CharSequence>) reader.next(key);
      assertNotNull(key);
      assertEquals("one", key.datum().toString());
      value = (AIngleValue<Integer>) reader.getCurrentValue(value);
      assertNotNull(value);
      assertEquals(1, value.datum().intValue());

      // Read the second record.
      key = (AIngleKey<CharSequence>) reader.next(key);
      assertNotNull(key);
      assertEquals("two", key.datum().toString());
      value = (AIngleValue<Integer>) reader.getCurrentValue(value);
      assertNotNull(value);
      assertEquals(2, value.datum().intValue());

      assertNull("Should be no more records.", reader.next(key));
    }
  }

  /** Tests that reading and writing ordinary Writables still works. */
  @Test
  public void testReadWritables() throws IOException {
    Path sequenceFilePath = new Path(new File(mTempDir.getRoot(), "output.seq").getPath());

    writeSequenceFile(sequenceFilePath, Text.class, IntWritable.class, null, null, new Text("one"), new IntWritable(1),
        new Text("two"), new IntWritable(2));

    Configuration conf = new Configuration();
    FileSystem fs = FileSystem.get(conf);
    AIngleSequenceFile.Reader.Options options = new AIngleSequenceFile.Reader.Options().withFileSystem(fs)
        .withInputPath(sequenceFilePath).withConfiguration(conf);

    try (SequenceFile.Reader reader = new AIngleSequenceFile.Reader(options)) {
      Text key = new Text();
      IntWritable value = new IntWritable();

      // Read the first record.
      assertTrue(reader.next(key));
      assertEquals("one", key.toString());
      reader.getCurrentValue(value);
      assertNotNull(value);
      assertEquals(1, value.get());

      // Read the second record.
      assertTrue(reader.next(key));
      assertEquals("two", key.toString());
      reader.getCurrentValue(value);
      assertNotNull(value);
      assertEquals(2, value.get());

      assertFalse("Should be no more records.", reader.next(key));

    }
  }

  /**
   * Writes a sequence file of records.
   *
   * @param file        The target file path.
   * @param keySchema   The schema of the key if using AIngle, else null.
   * @param valueSchema The schema of the value if using AIngle, else null.
   * @param records     <i>key1</i>, <i>value1</i>, <i>key2</i>, <i>value2</i>,
   *                    ...
   */
  private void writeSequenceFile(Path file, Class<?> keyClass, Class<?> valueClass, Schema keySchema,
      Schema valueSchema, Object... records) throws IOException {
    // Make sure the key/value records have an even size.
    if (0 != records.length % 2) {
      throw new IllegalArgumentException("Expected a value for each key record.");
    }

    // Open a AIngleSequenceFile writer.
    Configuration conf = new Configuration();
    FileSystem fs = FileSystem.get(conf);
    AIngleSequenceFile.Writer.Options options = new AIngleSequenceFile.Writer.Options().withFileSystem(fs)
        .withConfiguration(conf).withOutputPath(file);
    if (null != keySchema) {
      options.withKeySchema(keySchema);
    } else {
      options.withKeyClass(keyClass);
    }
    if (null != valueSchema) {
      options.withValueSchema(valueSchema);
    } else {
      options.withValueClass(valueClass);
    }
    try (SequenceFile.Writer writer = new AIngleSequenceFile.Writer(options)) {
      // Write some records.
      for (int i = 0; i < records.length; i += 2) {
        writer.append(records[i], records[i + 1]);
      }
    }
  }
}
