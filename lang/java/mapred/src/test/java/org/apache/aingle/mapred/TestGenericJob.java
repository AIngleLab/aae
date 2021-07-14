/*

 */
package org.apache.aingle.mapred;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Field;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.generic.GenericData;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;

import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.TextInputFormat;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

@SuppressWarnings("deprecation")
public class TestGenericJob {
  @Rule
  public TemporaryFolder DIR = new TemporaryFolder();

  private static Schema createSchema() {
    List<Field> fields = new ArrayList<>();

    fields.add(new Field("Optional", createArraySchema(), "", new ArrayList<>()));

    Schema recordSchema = Schema.createRecord("Container", "", "org.apache.aingle.mapred", false);
    recordSchema.setFields(fields);
    return recordSchema;
  }

  private static Schema createArraySchema() {
    List<Schema> schemas = new ArrayList<>();
    for (int i = 0; i < 5; i++) {
      schemas.add(createInnerSchema("optional_field_" + i));
    }

    Schema unionSchema = Schema.createUnion(schemas);
    return Schema.createArray(unionSchema);
  }

  private static Schema createInnerSchema(String name) {
    Schema innerrecord = Schema.createRecord(name, "", "", false);
    innerrecord.setFields(Collections.singletonList(new Field(name, Schema.create(Type.LONG), "", 0L)));
    return innerrecord;
  }

  @Before
  public void setup() throws IOException {
    // needed to satisfy the framework only - input ignored in mapper
    String dir = DIR.getRoot().getPath();
    File infile = new File(dir + "/in");
    RandomAccessFile file = new RandomAccessFile(infile, "rw");
    // add some data so framework actually calls our mapper
    file.writeChars("aa bb cc\ndd ee ff\n");
    file.close();
  }

  static class AIngleTestConverter extends MapReduceBase
      implements Mapper<LongWritable, Text, AIngleWrapper<Pair<Long, GenericData.Record>>, NullWritable> {

    public void map(LongWritable key, Text value,
        OutputCollector<AIngleWrapper<Pair<Long, GenericData.Record>>, NullWritable> out, Reporter reporter)
        throws IOException {
      GenericData.Record optional_entry = new GenericData.Record(createInnerSchema("optional_field_1"));
      optional_entry.put("optional_field_1", 0L);
      GenericData.Array<GenericData.Record> array = new GenericData.Array<>(1, createArraySchema());
      array.add(optional_entry);

      GenericData.Record container = new GenericData.Record(createSchema());
      container.put("Optional", array);

      out.collect(new AIngleWrapper<>(new Pair<>(key.get(), container)), NullWritable.get());
    }
  }

  @Test
  public void testJob() throws Exception {
    JobConf job = new JobConf();
    Path outputPath = new Path(DIR.getRoot().getPath() + "/out");
    outputPath.getFileSystem(job).delete(outputPath);

    job.setInputFormat(TextInputFormat.class);
    FileInputFormat.setInputPaths(job, DIR.getRoot().getPath() + "/in");

    job.setMapperClass(AIngleTestConverter.class);
    job.setNumReduceTasks(0);

    FileOutputFormat.setOutputPath(job, outputPath);
    System.out.println(createSchema());
    AIngleJob.setOutputSchema(job, Pair.getPairSchema(Schema.create(Schema.Type.LONG), createSchema()));
    job.setOutputFormat(AIngleOutputFormat.class);

    JobClient.runJob(job);
  }
}
