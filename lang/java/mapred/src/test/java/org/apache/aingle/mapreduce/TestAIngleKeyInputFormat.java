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

import static org.junit.Assert.*;
import static org.easymock.EasyMock.*;

import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.junit.Test;

public class TestAIngleKeyInputFormat {
  /**
   * Verifies that a non-null record reader can be created, and the key/value
   * types are as expected.
   */
  @Test
  public void testCreateRecordReader() throws IOException, InterruptedException {
    // Set up the job configuration.
    Job job = Job.getInstance();
    AIngleJob.setInputKeySchema(job, Schema.create(Schema.Type.STRING));
    Configuration conf = job.getConfiguration();

    FileSplit inputSplit = createMock(FileSplit.class);
    TaskAttemptContext context = createMock(TaskAttemptContext.class);
    expect(context.getConfiguration()).andReturn(conf).anyTimes();

    replay(inputSplit);
    replay(context);

    AIngleKeyInputFormat inputFormat = new AIngleKeyInputFormat();
    @SuppressWarnings("unchecked")
    RecordReader<AIngleKey<Object>, NullWritable> recordReader = inputFormat.createRecordReader(inputSplit, context);
    assertNotNull(inputFormat);
    recordReader.close();

    verify(inputSplit);
    verify(context);
  }
}
