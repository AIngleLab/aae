/*

 */
package org.apache.aingle.mapred;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.File;
import java.net.URI;
import java.util.Iterator;

import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.SequenceFileInputFormat;
import org.apache.hadoop.mapred.SequenceFileOutputFormat;

import org.apache.aingle.Schema;
import org.apache.aingle.file.FileReader;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.util.Utf8;

import org.junit.BeforeClass;
import org.junit.ClassRule;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestSequenceFileReader {

  private static final int COUNT = Integer.parseInt(System.getProperty("test.count", "10"));

  @ClassRule
  public static TemporaryFolder INPUT_DIR = new TemporaryFolder();

  @Rule
  public TemporaryFolder OUTPUT_DIR = new TemporaryFolder();

  public static File file() {
    return new File(INPUT_DIR.getRoot().getPath(), "test.seq");
  }

  private static final Schema SCHEMA = Pair.getPairSchema(Schema.create(Schema.Type.LONG),
      Schema.create(Schema.Type.STRING));

  @BeforeClass
  public static void testWriteSequenceFile() throws IOException {
    Configuration c = new Configuration();
    URI uri = file().toURI();
    try (SequenceFile.Writer writer = new SequenceFile.Writer(FileSystem.get(uri, c), c, new Path(uri.toString()),
        LongWritable.class, Text.class)) {
      final LongWritable key = new LongWritable();
      final Text val = new Text();
      for (int i = 0; i < COUNT; ++i) {
        key.set(i);
        val.set(Integer.toString(i));
        writer.append(key, val);
      }
    }
  }

  @Test
  public void testReadSequenceFile() throws Exception {
    checkFile(new SequenceFileReader<>(file()));
  }

  public void checkFile(FileReader<Pair<Long, CharSequence>> reader) throws Exception {
    long i = 0;
    for (Pair<Long, CharSequence> p : reader) {
      assertEquals((Long) i, p.key());
      assertEquals(Long.toString(i), p.value().toString());
      i++;
    }
    assertEquals(COUNT, i);
    reader.close();
  }

  @Test
  public void testSequenceFileInputFormat() throws Exception {
    JobConf job = new JobConf();
    Path outputPath = new Path(OUTPUT_DIR.getRoot().getPath());
    outputPath.getFileSystem(job).delete(outputPath, true);

    // configure input for AIngle from sequence file
    AIngleJob.setInputSequenceFile(job);
    FileInputFormat.setInputPaths(job, file().toURI().toString());
    AIngleJob.setInputSchema(job, SCHEMA);

    // mapper is default, identity
    // reducer is default, identity

    // configure output for aingle
    AIngleJob.setOutputSchema(job, SCHEMA);
    FileOutputFormat.setOutputPath(job, outputPath);

    JobClient.runJob(job);

    checkFile(new DataFileReader<>(new File(outputPath.toString() + "/part-00000.aingle"), new SpecificDatumReader<>()));
  }

  private static class NonAIngleMapper extends MapReduceBase
      implements Mapper<LongWritable, Text, AIngleKey<Long>, AIngleValue<Utf8>> {

    public void map(LongWritable key, Text value, OutputCollector<AIngleKey<Long>, AIngleValue<Utf8>> out,
        Reporter reporter) throws IOException {
      out.collect(new AIngleKey<>(key.get()), new AIngleValue<>(new Utf8(value.toString())));
    }
  }

  @Test
  public void testNonAIngleMapper() throws Exception {
    JobConf job = new JobConf();
    Path outputPath = new Path(OUTPUT_DIR.getRoot().getPath());
    outputPath.getFileSystem(job).delete(outputPath, true);

    // configure input for non-AIngle sequence file
    job.setInputFormat(SequenceFileInputFormat.class);
    FileInputFormat.setInputPaths(job, file().toURI().toString());

    // use a hadoop mapper that emits AIngle output
    job.setMapperClass(NonAIngleMapper.class);

    // reducer is default, identity

    // configure output for aingle
    FileOutputFormat.setOutputPath(job, outputPath);
    AIngleJob.setOutputSchema(job, SCHEMA);

    JobClient.runJob(job);

    checkFile(new DataFileReader<>(new File(outputPath.toString() + "/part-00000.aingle"), new SpecificDatumReader<>()));
  }

  private static class NonAIngleOnlyMapper extends MapReduceBase
      implements Mapper<LongWritable, Text, AIngleWrapper<Pair<Long, Utf8>>, NullWritable> {

    public void map(LongWritable key, Text value, OutputCollector<AIngleWrapper<Pair<Long, Utf8>>, NullWritable> out,
        Reporter reporter) throws IOException {
      out.collect(new AIngleWrapper<>(new Pair<>(key.get(), new Utf8(value.toString()))), NullWritable.get());
    }
  }

  @Test
  public void testNonAIngleMapOnly() throws Exception {
    JobConf job = new JobConf();
    Path outputPath = new Path(OUTPUT_DIR.getRoot().getPath());
    outputPath.getFileSystem(job).delete(outputPath, true);

    // configure input for non-AIngle sequence file
    job.setInputFormat(SequenceFileInputFormat.class);
    FileInputFormat.setInputPaths(job, file().toURI().toString());

    // use a hadoop mapper that emits AIngle output
    job.setMapperClass(NonAIngleOnlyMapper.class);

    // configure output for aingle
    job.setNumReduceTasks(0); // map-only
    FileOutputFormat.setOutputPath(job, outputPath);
    AIngleJob.setOutputSchema(job, SCHEMA);

    JobClient.runJob(job);

    checkFile(new DataFileReader<>(new File(outputPath.toString() + "/part-00000.aingle"), new SpecificDatumReader<>()));
  }

  private static class NonAIngleReducer extends MapReduceBase
      implements Reducer<AIngleKey<Long>, AIngleValue<Utf8>, LongWritable, Text> {

    public void reduce(AIngleKey<Long> key, Iterator<AIngleValue<Utf8>> values, OutputCollector<LongWritable, Text> out,
        Reporter reporter) throws IOException {
      while (values.hasNext()) {
        AIngleValue<Utf8> value = values.next();
        out.collect(new LongWritable(key.datum()), new Text(value.datum().toString()));
      }
    }
  }

  @Test
  public void testNonAIngleReducer() throws Exception {
    JobConf job = new JobConf();
    Path outputPath = new Path(OUTPUT_DIR.getRoot().getPath());
    outputPath.getFileSystem(job).delete(outputPath, true);

    // configure input for AIngle from sequence file
    AIngleJob.setInputSequenceFile(job);
    AIngleJob.setInputSchema(job, SCHEMA);
    FileInputFormat.setInputPaths(job, file().toURI().toString());

    // mapper is default, identity

    // use a hadoop reducer that consumes AIngle input
    AIngleJob.setMapOutputSchema(job, SCHEMA);
    job.setReducerClass(NonAIngleReducer.class);

    // configure outputPath for non-AIngle SequenceFile
    job.setOutputFormat(SequenceFileOutputFormat.class);
    FileOutputFormat.setOutputPath(job, outputPath);

    // output key/value classes are default, LongWritable/Text

    JobClient.runJob(job);

    checkFile(new SequenceFileReader<>(new File(outputPath.toString() + "/part-00000")));
  }

}
