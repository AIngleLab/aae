/*

 */

package org.apache.aingle.mapred;

import java.io.File;
import java.io.IOException;
import java.util.StringTokenizer;
import org.apache.aingle.Schema;
import org.apache.aingle.util.Utf8;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.Reporter;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestAIngleMultipleOutputs {

  @Rule
  public TemporaryFolder INPUT_DIR = new TemporaryFolder();

  @Rule
  public TemporaryFolder OUTPUT_DIR = new TemporaryFolder();

  public static class MapImpl extends AIngleMapper<Utf8, Pair<Utf8, Long>> {
    private AIngleMultipleOutputs amos;

    public void configure(JobConf Job) {
      this.amos = new AIngleMultipleOutputs(Job);
    }

    @Override
    public void map(Utf8 text, AIngleCollector<Pair<Utf8, Long>> collector, Reporter reporter) throws IOException {
      StringTokenizer tokens = new StringTokenizer(text.toString());
      while (tokens.hasMoreTokens()) {
        String tok = tokens.nextToken();
        collector.collect(new Pair<>(new Utf8(tok), 1L));
        amos.getCollector("myaingle2", reporter).collect(new Pair<Utf8, Long>(new Utf8(tok), 1L).toString());
      }
    }

    public void close() throws IOException {
      amos.close();
    }
  }

  public static class ReduceImpl extends AIngleReducer<Utf8, Long, Pair<Utf8, Long>> {
    private AIngleMultipleOutputs amos;

    public void configure(JobConf Job) {
      amos = new AIngleMultipleOutputs(Job);
    }

    @Override
    public void reduce(Utf8 word, Iterable<Long> counts, AIngleCollector<Pair<Utf8, Long>> collector, Reporter reporter)
        throws IOException {
      long sum = 0;
      for (long count : counts)
        sum += count;
      Pair<Utf8, Long> outputvalue = new Pair<>(word, sum);
      amos.getCollector("myaingle", reporter).collect(outputvalue);
      amos.collect("myaingle1", reporter, outputvalue.toString());
      amos.collect("myaingle", reporter, new Pair<Utf8, Long>(new Utf8(""), 0L).getSchema(), outputvalue, "testainglefile");
      amos.collect("myaingle", reporter, Schema.create(Schema.Type.STRING), outputvalue.toString(), "testainglefile1");
      collector.collect(new Pair<>(word, sum));
    }

    public void close() throws IOException {
      amos.close();
    }
  }

  @Test
  public void runTestsInOrder() throws Exception {
    String ainglePath = OUTPUT_DIR.getRoot().getPath();
    testJob(ainglePath);
    testProjection(ainglePath);
    testProjectionNewMethodsOne(ainglePath);
    testProjectionNewMethodsTwo(ainglePath);
    testProjection1(ainglePath);
    testJobNoreducer();
    testProjectionNoreducer(ainglePath);
  }

  @SuppressWarnings("deprecation")
  public void testJob(String pathOut) throws Exception {
    JobConf job = new JobConf();

    String pathIn = INPUT_DIR.getRoot().getPath();

    File fileIn = new File(pathIn, "lines.aingle");
    Path outputPath = new Path(pathOut);

    outputPath.getFileSystem(job).delete(outputPath);

    WordCountUtil.writeLinesFile(fileIn);

    job.setJobName("AIngleMultipleOutputs");

    AIngleJob.setInputSchema(job, Schema.create(Schema.Type.STRING));
    AIngleJob.setOutputSchema(job, new Pair<Utf8, Long>(new Utf8(""), 0L).getSchema());

    AIngleJob.setMapperClass(job, MapImpl.class);
    AIngleJob.setReducerClass(job, ReduceImpl.class);

    FileInputFormat.setInputPaths(job, pathIn);
    FileOutputFormat.setOutputPath(job, outputPath);
    FileOutputFormat.setCompressOutput(job, false);
    AIngleMultipleOutputs.addNamedOutput(job, "myaingle", AIngleOutputFormat.class,
        new Pair<Utf8, Long>(new Utf8(""), 0L).getSchema());
    AIngleMultipleOutputs.addNamedOutput(job, "myaingle1", AIngleOutputFormat.class, Schema.create(Schema.Type.STRING));
    AIngleMultipleOutputs.addNamedOutput(job, "myaingle2", AIngleOutputFormat.class, Schema.create(Schema.Type.STRING));
    WordCountUtil.setMeta(job);

    JobClient.runJob(job);

    WordCountUtil.validateCountsFile(new File(outputPath.toString(), "/part-00000.aingle"));
  }

  @SuppressWarnings("deprecation")
  public void testProjection(String inputDirectory) throws Exception {
    JobConf job = new JobConf();

    Integer defaultRank = -1;

    String jsonSchema = "{\"type\":\"record\"," + "\"name\":\"org.apache.aingle.mapred.Pair\"," + "\"fields\": [ "
        + "{\"name\":\"rank\", \"type\":\"int\", \"default\": -1}," + "{\"name\":\"value\", \"type\":\"long\"}" + "]}";

    Schema readerSchema = Schema.parse(jsonSchema);

    AIngleJob.setInputSchema(job, readerSchema);

    Path inputPath = new Path(inputDirectory + "/myaingle-r-00000.aingle");
    FileStatus fileStatus = FileSystem.get(job).getFileStatus(inputPath);
    FileSplit fileSplit = new FileSplit(inputPath, 0, fileStatus.getLen(), job);

    AIngleRecordReader<Pair<Integer, Long>> recordReader = new AIngleRecordReader<>(job, fileSplit);

    AIngleWrapper<Pair<Integer, Long>> inputPair = new AIngleWrapper<>(null);
    NullWritable ignore = NullWritable.get();

    long sumOfCounts = 0;
    long numOfCounts = 0;
    while (recordReader.next(inputPair, ignore)) {
      Assert.assertEquals(inputPair.datum().get(0), defaultRank);
      sumOfCounts += (Long) inputPair.datum().get(1);
      numOfCounts++;
    }

    Assert.assertEquals(numOfCounts, WordCountUtil.COUNTS.size());

    long actualSumOfCounts = 0;
    for (Long count : WordCountUtil.COUNTS.values()) {
      actualSumOfCounts += count;
    }

    Assert.assertEquals(sumOfCounts, actualSumOfCounts);
  }

  @SuppressWarnings("deprecation")
  public void testProjectionNewMethodsOne(String inputDirectory) throws Exception {
    JobConf job = new JobConf();

    Integer defaultRank = -1;

    String jsonSchema = "{\"type\":\"record\"," + "\"name\":\"org.apache.aingle.mapred.Pair\"," + "\"fields\": [ "
        + "{\"name\":\"rank\", \"type\":\"int\", \"default\": -1}," + "{\"name\":\"value\", \"type\":\"long\"}" + "]}";

    Schema readerSchema = Schema.parse(jsonSchema);

    AIngleJob.setInputSchema(job, readerSchema);

    Path inputPath = new Path(inputDirectory + "/testainglefile-r-00000.aingle");
    FileStatus fileStatus = FileSystem.get(job).getFileStatus(inputPath);
    FileSplit fileSplit = new FileSplit(inputPath, 0, fileStatus.getLen(), job);

    AIngleRecordReader<Pair<Integer, Long>> recordReader = new AIngleRecordReader<>(job, fileSplit);

    AIngleWrapper<Pair<Integer, Long>> inputPair = new AIngleWrapper<>(null);
    NullWritable ignore = NullWritable.get();

    long sumOfCounts = 0;
    long numOfCounts = 0;
    while (recordReader.next(inputPair, ignore)) {
      Assert.assertEquals(inputPair.datum().get(0), defaultRank);
      sumOfCounts += (Long) inputPair.datum().get(1);
      numOfCounts++;
    }

    Assert.assertEquals(numOfCounts, WordCountUtil.COUNTS.size());

    long actualSumOfCounts = 0;
    for (Long count : WordCountUtil.COUNTS.values()) {
      actualSumOfCounts += count;
    }

    Assert.assertEquals(sumOfCounts, actualSumOfCounts);

  }

  @SuppressWarnings("deprecation")
  // Test for a different schema output
  public void testProjection1(String inputDirectory) throws Exception {
    JobConf job = new JobConf();
    Schema readerSchema = Schema.create(Schema.Type.STRING);
    AIngleJob.setInputSchema(job, readerSchema);

    Path inputPath = new Path(inputDirectory + "/myaingle1-r-00000.aingle");
    FileStatus fileStatus = FileSystem.get(job).getFileStatus(inputPath);
    FileSplit fileSplit = new FileSplit(inputPath, 0, fileStatus.getLen(), job);
    AIngleWrapper<Utf8> inputPair = new AIngleWrapper<>(null);
    NullWritable ignore = NullWritable.get();
    AIngleRecordReader<Utf8> recordReader = new AIngleRecordReader<>(job, fileSplit);
    long sumOfCounts = 0;
    long numOfCounts = 0;
    while (recordReader.next(inputPair, ignore)) {
      sumOfCounts += Long.parseLong(inputPair.datum().toString().split(":")[2].replace("}", "").trim());
      numOfCounts++;
    }
    Assert.assertEquals(numOfCounts, WordCountUtil.COUNTS.size());
    long actualSumOfCounts = 0;
    for (Long count : WordCountUtil.COUNTS.values()) {
      actualSumOfCounts += count;
    }
    Assert.assertEquals(sumOfCounts, actualSumOfCounts);
  }

  @SuppressWarnings("deprecation")
  // Test for a different schema output
  public void testProjectionNewMethodsTwo(String inputDirectory) throws Exception {
    JobConf job = new JobConf();
    Schema readerSchema = Schema.create(Schema.Type.STRING);
    AIngleJob.setInputSchema(job, readerSchema);

    Path inputPath = new Path(inputDirectory + "/testainglefile1-r-00000.aingle");
    FileStatus fileStatus = FileSystem.get(job).getFileStatus(inputPath);
    FileSplit fileSplit = new FileSplit(inputPath, 0, fileStatus.getLen(), job);
    AIngleWrapper<Utf8> inputPair = new AIngleWrapper<>(null);
    NullWritable ignore = NullWritable.get();
    AIngleRecordReader<Utf8> recordReader = new AIngleRecordReader<>(job, fileSplit);
    long sumOfCounts = 0;
    long numOfCounts = 0;
    while (recordReader.next(inputPair, ignore)) {
      sumOfCounts += Long.parseLong(inputPair.datum().toString().split(":")[2].replace("}", "").trim());
      numOfCounts++;
    }
    Assert.assertEquals(numOfCounts, WordCountUtil.COUNTS.size());
    long actualSumOfCounts = 0;
    for (Long count : WordCountUtil.COUNTS.values()) {
      actualSumOfCounts += count;
    }
    Assert.assertEquals(sumOfCounts, actualSumOfCounts);
  }

  @SuppressWarnings("deprecation")
  public void testJobNoreducer() throws Exception {
    JobConf job = new JobConf();
    job.setNumReduceTasks(0);

    Path outputPath = new Path(OUTPUT_DIR.getRoot().getPath());
    outputPath.getFileSystem(job).delete(outputPath);

    WordCountUtil.writeLinesFile(new File(INPUT_DIR.getRoot(), "lines.aingle"));

    job.setJobName("AIngleMultipleOutputs_noreducer");

    AIngleJob.setInputSchema(job, Schema.create(Schema.Type.STRING));
    AIngleJob.setOutputSchema(job, new Pair<Utf8, Long>(new Utf8(""), 0L).getSchema());

    AIngleJob.setMapperClass(job, MapImpl.class);

    FileInputFormat.setInputPaths(job, new Path(INPUT_DIR.getRoot().toString()));
    FileOutputFormat.setOutputPath(job, outputPath);
    FileOutputFormat.setCompressOutput(job, false);
    AIngleMultipleOutputs.addNamedOutput(job, "myaingle2", AIngleOutputFormat.class, Schema.create(Schema.Type.STRING));
    JobClient.runJob(job);
  }

  public void testProjectionNoreducer(String inputDirectory) throws Exception {
    JobConf job = new JobConf();
    long onel = 1;
    Schema readerSchema = Schema.create(Schema.Type.STRING);
    AIngleJob.setInputSchema(job, readerSchema);
    Path inputPath = new Path(inputDirectory + "/myaingle2-m-00000.aingle");
    FileStatus fileStatus = FileSystem.get(job).getFileStatus(inputPath);
    FileSplit fileSplit = new FileSplit(inputPath, 0, fileStatus.getLen(), (String[]) null);
    AIngleRecordReader<Utf8> recordReader = new AIngleRecordReader<>(job, fileSplit);
    AIngleWrapper<Utf8> inputPair = new AIngleWrapper<>(null);
    NullWritable ignore = NullWritable.get();
    while (recordReader.next(inputPair, ignore)) {
      long testl = Long.parseLong(inputPair.datum().toString().split(":")[2].replace("}", "").trim());
      Assert.assertEquals(onel, testl);
    }
  }
}
