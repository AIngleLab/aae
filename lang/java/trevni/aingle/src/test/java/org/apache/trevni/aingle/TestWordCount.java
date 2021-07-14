/*

 */

package org.apache.trevni.aingle;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.lib.NullOutputFormat;
import org.apache.hadoop.mapred.Reporter;

import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.mapred.AIngleJob;
import org.apache.aingle.mapred.Pair;
import org.apache.aingle.mapred.AIngleMapper;
import org.apache.aingle.mapred.AIngleReducer;
import org.apache.aingle.mapred.AIngleCollector;

import org.apache.aingle.Schema;

import org.junit.Test;
import static org.junit.Assert.*;

public class TestWordCount {

  public static class MapImpl extends AIngleMapper<String, Pair<String, Long>> {
    @Override
    public void map(String text, AIngleCollector<Pair<String, Long>> collector, Reporter reporter) throws IOException {
      StringTokenizer tokens = new StringTokenizer(text);
      while (tokens.hasMoreTokens())
        collector.collect(new Pair<>(tokens.nextToken(), 1L));
    }
  }

  public static class ReduceImpl extends AIngleReducer<String, Long, Pair<String, Long>> {
    @Override
    public void reduce(String word, Iterable<Long> counts, AIngleCollector<Pair<String, Long>> collector,
        Reporter reporter) throws IOException {
      long sum = 0;
      for (long count : counts)
        sum += count;
      collector.collect(new Pair<>(word, sum));
    }
  }

  @Test
  public void runTestsInOrder() throws Exception {
    testOutputFormat();
    testInputFormat();
  }

  static final Schema STRING = Schema.create(Schema.Type.STRING);
  static {
    GenericData.setStringType(STRING, GenericData.StringType.String);
  }
  static final Schema LONG = Schema.create(Schema.Type.LONG);

  public void testOutputFormat() throws Exception {
    JobConf job = new JobConf();

    WordCountUtil wordCountUtil = new WordCountUtil("trevniMapredTest");

    wordCountUtil.writeLinesFile();

    AIngleJob.setInputSchema(job, STRING);
    AIngleJob.setOutputSchema(job, Pair.getPairSchema(STRING, LONG));

    AIngleJob.setMapperClass(job, MapImpl.class);
    AIngleJob.setCombinerClass(job, ReduceImpl.class);
    AIngleJob.setReducerClass(job, ReduceImpl.class);

    FileInputFormat.setInputPaths(job, new Path(wordCountUtil.getDir().toString() + "/in"));
    FileOutputFormat.setOutputPath(job, new Path(wordCountUtil.getDir().toString() + "/out"));
    FileOutputFormat.setCompressOutput(job, true);

    job.setOutputFormat(AIngleTrevniOutputFormat.class);

    JobClient.runJob(job);

    wordCountUtil.validateCountsFile();
  }

  private static long total;

  public static class Counter extends AIngleMapper<GenericRecord, Void> {
    @Override
    public void map(GenericRecord r, AIngleCollector<Void> collector, Reporter reporter) throws IOException {
      total += (Long) r.get("value");
    }
  }

  public void testInputFormat() throws Exception {
    JobConf job = new JobConf();

    WordCountUtil wordCountUtil = new WordCountUtil("trevniMapredTest");

    Schema subSchema = new Schema.Parser().parse("{\"type\":\"record\"," + "\"name\":\"PairValue\"," + "\"fields\": [ "
        + "{\"name\":\"value\", \"type\":\"long\"}" + "]}");
    AIngleJob.setInputSchema(job, subSchema);
    AIngleJob.setMapperClass(job, Counter.class);
    FileInputFormat.setInputPaths(job, new Path(wordCountUtil.getDir().toString() + "/out/*"));
    job.setInputFormat(AIngleTrevniInputFormat.class);

    job.setNumReduceTasks(0); // map-only
    job.setOutputFormat(NullOutputFormat.class); // ignore output

    total = 0;
    JobClient.runJob(job);
    assertEquals(WordCountUtil.TOTAL, total);
  }

}
