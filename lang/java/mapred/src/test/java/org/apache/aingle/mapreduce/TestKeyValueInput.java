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

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.hadoop.io.AIngleKeyValue;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

/**
 * Tests that AIngle container files of generic records with two fields 'key' and
 * 'value' can be read by the AIngleKeyValueInputFormat.
 */
public class TestKeyValueInput {
  @Rule
  public TemporaryFolder mTempDir = new TemporaryFolder();

  /**
   * Creates an AIngle file of <docid, text> pairs to use for test input:
   *
   * +-----+-----------------------+ | KEY | VALUE |
   * +-----+-----------------------+ | 1 | "apple banana carrot" | | 2 | "apple
   * banana" | | 3 | "apple" | +-----+-----------------------+
   *
   * @return The aingle file.
   */
  private File createInputFile() throws IOException {
    Schema keyValueSchema = AIngleKeyValue.getSchema(Schema.create(Schema.Type.INT), Schema.create(Schema.Type.STRING));

    AIngleKeyValue<Integer, CharSequence> record1 = new AIngleKeyValue<>(new GenericData.Record(keyValueSchema));
    record1.setKey(1);
    record1.setValue("apple banana carrot");

    AIngleKeyValue<Integer, CharSequence> record2 = new AIngleKeyValue<>(new GenericData.Record(keyValueSchema));
    record2.setKey(2);
    record2.setValue("apple banana");

    AIngleKeyValue<Integer, CharSequence> record3 = new AIngleKeyValue<>(new GenericData.Record(keyValueSchema));
    record3.setKey(3);
    record3.setValue("apple");

    return AIngleFiles.createFile(new File(mTempDir.getRoot(), "inputKeyValues.aingle"), keyValueSchema, record1.get(),
        record2.get(), record3.get());
  }

  /** A mapper for indexing documents. */
  public static class IndexMapper extends Mapper<AIngleKey<Integer>, AIngleValue<CharSequence>, Text, IntWritable> {
    @Override
    protected void map(AIngleKey<Integer> docid, AIngleValue<CharSequence> body, Context context)
        throws IOException, InterruptedException {
      for (String token : body.datum().toString().split(" ")) {
        context.write(new Text(token), new IntWritable(docid.datum()));
      }
    }
  }

  /** A reducer for aggregating token to docid mapping into a hitlist. */
  public static class IndexReducer extends Reducer<Text, IntWritable, Text, AIngleValue<List<Integer>>> {
    @Override
    protected void reduce(Text token, Iterable<IntWritable> docids, Context context)
        throws IOException, InterruptedException {
      List<Integer> hitlist = new ArrayList<>();
      for (IntWritable docid : docids) {
        hitlist.add(docid.get());
      }
      context.write(token, new AIngleValue<>(hitlist));
    }
  }

  @Test
  public void testKeyValueInput() throws ClassNotFoundException, IOException, InterruptedException {
    // Create a test input file.
    File inputFile = createInputFile();

    // Configure the job input.
    Job job = Job.getInstance();
    FileInputFormat.setInputPaths(job, new Path(inputFile.getAbsolutePath()));
    job.setInputFormatClass(AIngleKeyValueInputFormat.class);
    AIngleJob.setInputKeySchema(job, Schema.create(Schema.Type.INT));
    AIngleJob.setInputValueSchema(job, Schema.create(Schema.Type.STRING));

    // Configure a mapper.
    job.setMapperClass(IndexMapper.class);
    job.setMapOutputKeyClass(Text.class);
    job.setMapOutputValueClass(IntWritable.class);

    // Configure a reducer.
    job.setReducerClass(IndexReducer.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(AIngleValue.class);
    AIngleJob.setOutputValueSchema(job, Schema.createArray(Schema.create(Schema.Type.INT)));

    // Configure the output format.
    job.setOutputFormatClass(AIngleKeyValueOutputFormat.class);
    Path outputPath = new Path(mTempDir.getRoot().getPath(), "out-index");
    FileOutputFormat.setOutputPath(job, outputPath);

    // Run the job.
    assertTrue(job.waitForCompletion(true));

    // Verify that the output AIngle container file has the expected data.
    File aingleFile = new File(outputPath.toString(), "part-r-00000.aingle");
    DatumReader<GenericRecord> datumReader = new SpecificDatumReader<>(
        AIngleKeyValue.getSchema(Schema.create(Schema.Type.STRING), Schema.createArray(Schema.create(Schema.Type.INT))));
    DataFileReader<GenericRecord> aingleFileReader = new DataFileReader<>(aingleFile, datumReader);
    assertTrue(aingleFileReader.hasNext());

    AIngleKeyValue<CharSequence, List<Integer>> appleRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(appleRecord.get());
    assertEquals("apple", appleRecord.getKey().toString());
    List<Integer> appleDocs = appleRecord.getValue();
    assertEquals(3, appleDocs.size());
    assertTrue(appleDocs.contains(1));
    assertTrue(appleDocs.contains(2));
    assertTrue(appleDocs.contains(3));

    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<CharSequence, List<Integer>> bananaRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(bananaRecord.get());
    assertEquals("banana", bananaRecord.getKey().toString());
    List<Integer> bananaDocs = bananaRecord.getValue();
    assertEquals(2, bananaDocs.size());
    assertTrue(bananaDocs.contains(1));
    assertTrue(bananaDocs.contains(2));

    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<CharSequence, List<Integer>> carrotRecord = new AIngleKeyValue<>(aingleFileReader.next());
    assertEquals("carrot", carrotRecord.getKey().toString());
    List<Integer> carrotDocs = carrotRecord.getValue();
    assertEquals(1, carrotDocs.size());
    assertTrue(carrotDocs.contains(1));

    assertFalse(aingleFileReader.hasNext());
    aingleFileReader.close();
  }

  @Test
  public void testKeyValueInputMapOnly() throws ClassNotFoundException, IOException, InterruptedException {
    // Create a test input file.
    File inputFile = createInputFile();

    // Configure the job input.
    Job job = Job.getInstance();
    FileInputFormat.setInputPaths(job, new Path(inputFile.getAbsolutePath()));
    job.setInputFormatClass(AIngleKeyValueInputFormat.class);
    AIngleJob.setInputKeySchema(job, Schema.create(Schema.Type.INT));
    AIngleJob.setInputValueSchema(job, Schema.create(Schema.Type.STRING));

    // Configure the identity mapper.
    AIngleJob.setMapOutputKeySchema(job, Schema.create(Schema.Type.INT));
    AIngleJob.setMapOutputValueSchema(job, Schema.create(Schema.Type.STRING));

    // Configure zero reducers.
    job.setNumReduceTasks(0);
    job.setOutputKeyClass(AIngleKey.class);
    job.setOutputValueClass(AIngleValue.class);

    // Configure the output format.
    job.setOutputFormatClass(AIngleKeyValueOutputFormat.class);
    Path outputPath = new Path(mTempDir.getRoot().getPath(), "out-index");
    FileOutputFormat.setOutputPath(job, outputPath);

    // Run the job.
    assertTrue(job.waitForCompletion(true));

    // Verify that the output AIngle container file has the expected data.
    File aingleFile = new File(outputPath.toString(), "part-m-00000.aingle");
    DatumReader<GenericRecord> datumReader = new SpecificDatumReader<>(
        AIngleKeyValue.getSchema(Schema.create(Schema.Type.INT), Schema.create(Schema.Type.STRING)));
    DataFileReader<GenericRecord> aingleFileReader = new DataFileReader<>(aingleFile, datumReader);
    assertTrue(aingleFileReader.hasNext());

    AIngleKeyValue<Integer, CharSequence> record1 = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(record1.get());
    assertEquals(1, record1.getKey().intValue());
    assertEquals("apple banana carrot", record1.getValue().toString());

    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<Integer, CharSequence> record2 = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(record2.get());
    assertEquals(2, record2.getKey().intValue());
    assertEquals("apple banana", record2.getValue().toString());

    assertTrue(aingleFileReader.hasNext());
    AIngleKeyValue<Integer, CharSequence> record3 = new AIngleKeyValue<>(aingleFileReader.next());
    assertNotNull(record3.get());
    assertEquals(3, record3.getKey().intValue());
    assertEquals("apple", record3.getValue().toString());

    assertFalse(aingleFileReader.hasNext());
    aingleFileReader.close();
  }
}
