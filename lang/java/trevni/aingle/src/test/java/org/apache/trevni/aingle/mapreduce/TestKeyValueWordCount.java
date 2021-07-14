/*

 */

package org.apache.trevni.aingle.mapreduce;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.aingle.mapreduce.AIngleJob;
import org.apache.aingle.mapreduce.AIngleKeyInputFormat;
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

public class TestKeyValueWordCount {

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

  private static class WordCountReducer extends Reducer<Text, LongWritable, AIngleKey<String>, AIngleValue<Long>> {

    AIngleKey<String> resultKey = new AIngleKey<>();
    AIngleValue<Long> resultValue = new AIngleValue<>();

    @Override
    protected void reduce(Text key, Iterable<LongWritable> values, Context context)
        throws IOException, InterruptedException {
      long sum = 0;
      for (LongWritable value : values) {
        sum += value.get();
      }
      resultKey.datum(key.toString());
      resultValue.datum(sum);

      context.write(resultKey, resultValue);
    }
  }

  public static class Counter extends Mapper<AIngleKey<String>, AIngleValue<Long>, NullWritable, NullWritable> {
    @Override
    protected void map(AIngleKey<String> key, AIngleValue<Long> value, Context context)
        throws IOException, InterruptedException {
      total += value.datum();
    }
  }

  @Test
  public void testIOFormat() throws Exception {
    checkOutputFormat();
    checkInputFormat();
  }

  public void checkOutputFormat() throws Exception {
    Job job = Job.getInstance();

    WordCountUtil wordCountUtil = new WordCountUtil("trevniMapReduceKeyValueTest", "part-r-00000");

    wordCountUtil.writeLinesFile();

    AIngleJob.setInputKeySchema(job, STRING);
    AIngleJob.setOutputKeySchema(job, STRING);
    AIngleJob.setOutputValueSchema(job, LONG);

    job.setMapperClass(WordCountMapper.class);
    job.setReducerClass(WordCountReducer.class);

    job.setMapOutputKeyClass(Text.class);
    job.setMapOutputValueClass(LongWritable.class);

    FileInputFormat.setInputPaths(job, new Path(wordCountUtil.getDir().toString() + "/in"));
    FileOutputFormat.setOutputPath(job, new Path(wordCountUtil.getDir().toString() + "/out"));
    FileOutputFormat.setCompressOutput(job, true);

    job.setInputFormatClass(AIngleKeyInputFormat.class);
    job.setOutputFormatClass(AIngleTrevniKeyValueOutputFormat.class);

    job.waitForCompletion(true);

    wordCountUtil.validateCountsFileGenericRecord();
  }

  public void checkInputFormat() throws Exception {
    Job job = Job.getInstance();

    WordCountUtil wordCountUtil = new WordCountUtil("trevniMapReduceKeyValueTest");

    job.setMapperClass(Counter.class);

    FileInputFormat.setInputPaths(job, new Path(wordCountUtil.getDir().toString() + "/out/*"));
    job.setInputFormatClass(AIngleTrevniKeyValueInputFormat.class);

    job.setNumReduceTasks(0);
    job.setOutputFormatClass(NullOutputFormat.class);

    total = 0;
    job.waitForCompletion(true);
    assertEquals(WordCountUtil.TOTAL, total);

  }
}
