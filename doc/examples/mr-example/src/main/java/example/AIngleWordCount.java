/*

 */

package example;

import java.io.IOException;
import java.util.*;

import org.apache.aingle.*;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.mapred.*;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

/**
 * The classic WordCount example modified to output AIngle Pair<CharSequence,
 * Integer> records instead of text.
 */
public class AIngleWordCount extends Configured implements Tool {

  public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, IntWritable> {
    private final static IntWritable one = new IntWritable(1);
    private Text word = new Text();

    public void map(LongWritable key, Text value, OutputCollector<Text, IntWritable> output, Reporter reporter)
        throws IOException {
      String line = value.toString();
      StringTokenizer tokenizer = new StringTokenizer(line);
      while (tokenizer.hasMoreTokens()) {
        word.set(tokenizer.nextToken());
        output.collect(word, one);
      }
    }
  }

  public static class Reduce extends MapReduceBase
    implements Reducer<Text, IntWritable,
                       AIngleWrapper<Pair<CharSequence, Integer>>, NullWritable> {

    public void reduce(Text key, Iterator<IntWritable> values,
        OutputCollector<AIngleWrapper<Pair<CharSequence, Integer>>, NullWritable> output,
        Reporter reporter) throws IOException {
      int sum = 0;
      while (values.hasNext()) {
        sum += values.next().get();
      }
      output.collect(new AIngleWrapper<Pair<CharSequence, Integer>>(
          new Pair<CharSequence, Integer>(key.toString(), sum)),
          NullWritable.get());
    }
  }

  public int run(String[] args) throws Exception {
    if (args.length != 2) {
      System.err.println("Usage: AIngleWordCount <input path> <output path>");
      return -1;
    }

    JobConf conf = new JobConf(AIngleWordCount.class);
    conf.setJobName("wordcount");

    // We call setOutputSchema first so we can override the configuration
    // parameters it sets
    AIngleJob.setOutputSchema(conf, Pair.getPairSchema(Schema.create(Type.STRING),
        Schema.create(Type.INT)));

    conf.setMapperClass(Map.class);
    conf.setReducerClass(Reduce.class);

    conf.setInputFormat(TextInputFormat.class);

    conf.setMapOutputKeyClass(Text.class);
    conf.setMapOutputValueClass(IntWritable.class);
    conf.setOutputKeyComparatorClass(Text.Comparator.class);

    FileInputFormat.setInputPaths(conf, new Path(args[0]));
    FileOutputFormat.setOutputPath(conf, new Path(args[1]));

    JobClient.runJob(conf);
    return 0;
  }

  public static void main(String[] args) throws Exception {
    int res = ToolRunner.run(new Configuration(), new AIngleWordCount(), args);
    System.exit(res);
  }
}
