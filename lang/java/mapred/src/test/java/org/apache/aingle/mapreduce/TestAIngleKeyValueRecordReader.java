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

import static org.easymock.EasyMock.*;
import static org.junit.Assert.*;

import java.io.File;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.file.SeekableFileInput;
import org.apache.aingle.file.SeekableInput;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.hadoop.io.AIngleKeyValue;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestAIngleKeyValueRecordReader {
  /** A temporary directory for test data. */
  @Rule
  public TemporaryFolder mTempDir = new TemporaryFolder();

  /**
   * Verifies that aingle records can be read and progress is reported correctly.
   */
  @Test
  public void testReadRecords() throws IOException, InterruptedException {
    // Create the test aingle file input with two records:
    // 1. <"firstkey", 1>
    // 2. <"second", 2>
    Schema keyValueSchema = AIngleKeyValue.getSchema(Schema.create(Schema.Type.STRING), Schema.create(Schema.Type.INT));

    AIngleKeyValue<CharSequence, Integer> firstInputRecord = new AIngleKeyValue<>(new GenericData.Record(keyValueSchema));
    firstInputRecord.setKey("first");
    firstInputRecord.setValue(1);

    AIngleKeyValue<CharSequence, Integer> secondInputRecord = new AIngleKeyValue<>(new GenericData.Record(keyValueSchema));
    secondInputRecord.setKey("second");
    secondInputRecord.setValue(2);

    final SeekableInput aingleFileInput = new SeekableFileInput(
        AIngleFiles.createFile(new File(mTempDir.getRoot(), "myInputFile.aingle"), keyValueSchema, firstInputRecord.get(),
            secondInputRecord.get()));

    // Create the record reader over the aingle input file.
    RecordReader<AIngleKey<CharSequence>, AIngleValue<Integer>> recordReader = new AIngleKeyValueRecordReader<CharSequence, Integer>(
        Schema.create(Schema.Type.STRING), Schema.create(Schema.Type.INT)) {
      @Override
      protected SeekableInput createSeekableInput(Configuration conf, Path path) throws IOException {
        return aingleFileInput;
      }
    };

    // Set up the job configuration.
    Configuration conf = new Configuration();

    // Create a mock input split for this record reader.
    FileSplit inputSplit = createMock(FileSplit.class);
    expect(inputSplit.getPath()).andReturn(new Path("/path/to/an/aingle/file")).anyTimes();
    expect(inputSplit.getStart()).andReturn(0L).anyTimes();
    expect(inputSplit.getLength()).andReturn(aingleFileInput.length()).anyTimes();

    // Create a mock task attempt context for this record reader.
    TaskAttemptContext context = createMock(TaskAttemptContext.class);
    expect(context.getConfiguration()).andReturn(conf).anyTimes();

    // Initialize the record reader.
    replay(inputSplit);
    replay(context);
    recordReader.initialize(inputSplit, context);

    assertEquals("Progress should be zero before any records are read", 0.0f, recordReader.getProgress(), 0.0f);

    // Some variables to hold the records.
    AIngleKey<CharSequence> key;
    AIngleValue<Integer> value;

    // Read the first record.
    assertTrue("Expected at least one record", recordReader.nextKeyValue());
    key = recordReader.getCurrentKey();
    value = recordReader.getCurrentValue();

    assertNotNull("First record had null key", key);
    assertNotNull("First record had null value", value);

    assertEquals("first", key.datum().toString());
    assertEquals(1, value.datum().intValue());

    assertTrue("getCurrentKey() returned different keys for the same record", key == recordReader.getCurrentKey());
    assertTrue("getCurrentValue() returned different values for the same record",
        value == recordReader.getCurrentValue());

    // Read the second record.
    assertTrue("Expected to read a second record", recordReader.nextKeyValue());
    key = recordReader.getCurrentKey();
    value = recordReader.getCurrentValue();

    assertNotNull("Second record had null key", key);
    assertNotNull("Second record had null value", value);

    assertEquals("second", key.datum().toString());
    assertEquals(2, value.datum().intValue());

    assertEquals("Progress should be complete (2 out of 2 records processed)", 1.0f, recordReader.getProgress(), 0.0f);

    // There should be no more records.
    assertFalse("Expected only 2 records", recordReader.nextKeyValue());

    // Close the record reader.
    recordReader.close();

    // Verify the expected calls on the mocks.
    verify(inputSplit);
    verify(context);
  }
}
