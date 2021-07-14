/*

 */

package org.apache.trevni.aingle.mapreduce;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericData.Record;
import org.apache.aingle.mapreduce.AIngleJob;
import org.apache.aingle.mapreduce.AIngleKeyInputFormat;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.Pair;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.NullOutputFormat;
import org.apache.trevni.aingle.WordCountUtil;
import org.junit.Test;

public class TestKeyWordCount {

  private static long total = 0;

  static final Schema STRING = Schema.create(Schema.Type.STRING);
  static {
    GenericData.setStringType(STRING, GenericData.StringType.String);
  }
  static final Schema LONG = Schema.create(Schema.Type.LONG);

  private static class WordCountMapper extends Mapper<AIngleKey<String>, NullWritable, Text, LongWritable> {
    private LongWritable mCount = new LongWritable();
    private Text mText = new Text();

    @Override
    protected void setup(Context context) {
      mCount.set(1);
    }

    @Override
    protected void map(AIngleKey<String> key, NullWritable value, Context context)
        throws IOException, InterruptedException {

      try {
        StringTokenizer tokens = new StringTokenizer(key.datum());
        while (tokens.hasMoreTokens()) {
          mText.set(tokens.nextToken());
          context.write(mText, mCount);
        }
      } catch (Exception e) {
        throw new RuntimeException(key + " " + key.datum(), e);
      }

    }
  }

  private static class WordCountReducer extends Reducer<Text, LongWritable, AIngleKey<GenericData.Record>, NullWritable> {

    private AIngleKey<GenericData.Record> result;

    @Override
    protected void setup(Context context) {
      result = new AIngleKey<>();
      result.datum(new Record(Pair.getPairSchema(STRING, LONG)));
    }

    @Override
    protected void reduce(Text key, Iterable<LongWritable> values, Context context)
        throws IOException, InterruptedException {
      long count = 0;
      for (LongWritable value : values) {
        count += value.get();
      }

      result.datum().put("key", key.toString());
      result.datum().put("value", count);

      context.write(result, NullWritable.get());
    }
  }

  public static class Counter extends Mapper<AIngleKey<GenericData.Record>, NullWritable, NullWritable, NullWritable> {
    @Override
    protected void map(AIngleKey<GenericData.Record> key, NullWritable value, Context context)
        throws IOException, InterruptedException {
      total += (Long) key.datum().get("value");
    }
  }

  @Test
  public void testIOFormat() throws Exception {
    checkOutputFormat();
    checkInputFormat();
  }

  public void checkOutputFormat() throws Exception {
    Job job = Job.getInstance();

    WordCountUtil wordCountUtil = new WordCountUtil("trevniMapReduceKeyTest", "part-r-00000");

    wordCountUtil.writeLinesFile();

    AIngleJob.setInputKeySchema(job, STRING);
    AIngleJob.setOutputKeySchema(job, Pair.getPairSchema(STRING, LONG));

    job.setMapperClass(WordCountMapper.class);
    job.setReducerClass(WordCountReducer.class);

    job.setMapOutputKeyClass(Text.class);
    job.setMapOutputValueClass(LongWritable.class);

    FileInputFormat.setInputPaths(job, new Path(wordCountUtil.getDir().toString() + "/in"));
    FileOutputFormat.setOutputPath(job, new Path(wordCountUtil.getDir().toString() + "/out"));
    FileOutputFormat.setCompressOutput(job, true);

    job.setInputFormatClass(AIngleKeyInputFormat.class);
    job.setOutputFormatClass(AIngleTrevniKeyOutputFormat.class);

    job.waitForCompletion(true);

    wordCountUtil.validateCountsFile();
  }

  public void checkInputFormat() throws Exception {
    Job job = Job.getInstance();

    WordCountUtil wordCountUtil = new WordCountUtil("trevniMapReduceKeyTest");

    job.setMapperClass(Counter.class);

    Schema subSchema = new Schema.Parser().parse("{\"type\":\"record\"," + "\"name\":\"PairValue\"," + "\"fields\": [ "
        + "{\"name\":\"value\", \"type\":\"long\"}" + "]}");
    AIngleJob.setInputKeySchema(job, subSchema);

    FileInputFormat.setInputPaths(job, new Path(wordCountUtil.getDir().toString() + "/out/*"));
    job.setInputFormatClass(AIngleTrevniKeyInputFormat.class);

    job.setNumReduceTasks(0);
    job.setOutputFormatClass(NullOutputFormat.class);

    total = 0;
    job.waitForCompletion(true);
    assertEquals(WordCountUtil.TOTAL, total);

  }

}
