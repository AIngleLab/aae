/*

 */

package org.apache.aingle.mapred;

import static org.junit.Assert.assertEquals;

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
import org.junit.ClassRule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestWordCount {

  @ClassRule
  public static TemporaryFolder INPUT_DIR = new TemporaryFolder();

  @ClassRule
  public static TemporaryFolder OUTPUT_DIR = new TemporaryFolder();

  public static class MapImpl extends AIngleMapper<Utf8, Pair<Utf8, Long>> {
    @Override
    public void map(Utf8 text, AIngleCollector<Pair<Utf8, Long>> collector, Reporter reporter) throws IOException {
      StringTokenizer tokens = new StringTokenizer(text.toString());
      while (tokens.hasMoreTokens())
        collector.collect(new Pair<>(new Utf8(tokens.nextToken()), 1L));
    }
  }

  public static class ReduceImpl extends AIngleReducer<Utf8, Long, Pair<Utf8, Long>> {
    @Override
    public void reduce(Utf8 word, Iterable<Long> counts, AIngleCollector<Pair<Utf8, Long>> collector, Reporter reporter)
        throws IOException {
      long sum = 0;
      for (long count : counts)
        sum += count;
      collector.collect(new Pair<>(word, sum));
    }
  }

  @Test
  public void runTestsInOrder() throws Exception {
    String pathOut = OUTPUT_DIR.getRoot().getPath();
    testJob(pathOut);
    testProjection(pathOut);
  }

  @SuppressWarnings("deprecation")
  public void testJob(String pathOut) throws Exception {
    JobConf job = new JobConf();
    String pathIn = INPUT_DIR.getRoot().getPath();

    WordCountUtil.writeLinesFile(pathIn + "/lines.aingle");

    Path outputPath = new Path(pathOut);
    outputPath.getFileSystem(job).delete(outputPath);

    job.setJobName("wordcount");

    AIngleJob.setInputSchema(job, Schema.create(Schema.Type.STRING));
    AIngleJob.setOutputSchema(job, new Pair<Utf8, Long>(new Utf8(""), 0L).getSchema());

    AIngleJob.setMapperClass(job, MapImpl.class);
    AIngleJob.setCombinerClass(job, ReduceImpl.class);
    AIngleJob.setReducerClass(job, ReduceImpl.class);

    FileInputFormat.setInputPaths(job, new Path(pathIn));
    FileOutputFormat.setOutputPath(job, new Path(pathOut));
    FileOutputFormat.setCompressOutput(job, true);

    WordCountUtil.setMeta(job);

    JobClient.runJob(job);

    WordCountUtil.validateCountsFile(new File(pathOut, "part-00000.aingle"));
  }

  @SuppressWarnings("deprecation")
  public void testProjection(String inputPathString) throws Exception {
    JobConf job = new JobConf();

    Integer defaultRank = -1;

    String jsonSchema = "{\"type\":\"record\"," + "\"name\":\"org.apache.aingle.mapred.Pair\"," + "\"fields\": [ "
        + "{\"name\":\"rank\", \"type\":\"int\", \"default\": -1}," + "{\"name\":\"value\", \"type\":\"long\"}" + "]}";

    Schema readerSchema = Schema.parse(jsonSchema);

    AIngleJob.setInputSchema(job, readerSchema);

    Path inputPath = new Path(inputPathString + "/part-00000.aingle");
    FileStatus fileStatus = FileSystem.get(job).getFileStatus(inputPath);
    FileSplit fileSplit = new FileSplit(inputPath, 0, fileStatus.getLen(), job);

    AIngleRecordReader<Pair<Integer, Long>> recordReader = new AIngleRecordReader<>(job, fileSplit);

    AIngleWrapper<Pair<Integer, Long>> inputPair = new AIngleWrapper<>(null);
    NullWritable ignore = NullWritable.get();

    long sumOfCounts = 0;
    long numOfCounts = 0;
    while (recordReader.next(inputPair, ignore)) {
      assertEquals(inputPair.datum().get(0), defaultRank);
      sumOfCounts += (Long) inputPair.datum().get(1);
      numOfCounts++;
    }

    assertEquals(numOfCounts, WordCountUtil.COUNTS.size());

    long actualSumOfCounts = 0;
    for (Long count : WordCountUtil.COUNTS.values()) {
      actualSumOfCounts += count;
    }

    assertEquals(sumOfCounts, actualSumOfCounts);
  }

}
