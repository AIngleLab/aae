/*

 */

package org.apache.aingle.mapreduce;

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
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

public class TestCombineAIngleKeyValueFileInputFormat {

  /** A temporary directory for test data. */
  @Rule
  public TemporaryFolder mTempDir = new TemporaryFolder();

  /**
   * Verifies that aingle records can be read in multi files.
   */
  @Test
  public void testReadRecords() throws IOException, InterruptedException, ClassNotFoundException {

    Schema keyValueSchema = AIngleKeyValue.getSchema(Schema.create(Schema.Type.INT), Schema.create(Schema.Type.STRING));

    AIngleKeyValue<Integer, CharSequence> record1 = new AIngleKeyValue<>(new GenericData.Record(keyValueSchema));
    record1.setKey(1);
    record1.setValue("apple banana carrot");
    AIngleFiles.createFile(new File(mTempDir.getRoot(), "combineSplit00.aingle"), keyValueSchema, record1.get());

    AIngleKeyValue<Integer, CharSequence> record2 = new AIngleKeyValue<>(new GenericData.Record(keyValueSchema));
    record2.setKey(2);
    record2.setValue("apple banana");

    AIngleFiles.createFile(new File(mTempDir.getRoot(), "combineSplit01.aingle"), keyValueSchema, record2.get());

    // Configure the job input.
    Job job = Job.getInstance();
    FileInputFormat.setInputPaths(job, new Path(mTempDir.getRoot().getAbsolutePath()));
    job.setInputFormatClass(CombineAIngleKeyValueFileInputFormat.class);
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
    Path outputPath = new Path(mTempDir.getRoot().getPath(), "out");
    FileOutputFormat.setOutputPath(job, outputPath);

    // Run the job.
    assertTrue(job.waitForCompletion(true));

    // Verify that the output AIngle container file has the expected data.
    File aingleFile = new File(outputPath.toString(), "part-m-00000.aingle");
    DatumReader<GenericRecord> datumReader = new SpecificDatumReader<>(
        AIngleKeyValue.getSchema(Schema.create(Schema.Type.INT), Schema.create(Schema.Type.STRING)));
    DataFileReader<GenericRecord> aingleFileReader = new DataFileReader<>(aingleFile, datumReader);
    assertTrue(aingleFileReader.hasNext());

    while (aingleFileReader.hasNext()) {
      AIngleKeyValue<Integer, CharSequence> mapRecord1 = new AIngleKeyValue<>(aingleFileReader.next());
      assertNotNull(mapRecord1.get());
      if (mapRecord1.getKey().intValue() == 1) {
        assertEquals("apple banana carrot", mapRecord1.getValue().toString());
      } else if (mapRecord1.getKey().intValue() == 2) {
        assertEquals("apple banana", mapRecord1.getValue().toString());
      } else {
        fail("Unknown key " + mapRecord1.getKey().intValue());
      }
    }
    aingleFileReader.close();
  }
}
