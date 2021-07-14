/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.util.StringTokenizer;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.Reporter;

import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.reflect.ReflectDatumWriter;
import org.apache.aingle.reflect.ReflectDatumReader;

import org.junit.Test;
import static org.junit.Assert.*;

public class TestReflectJob {

  /** The input class. */
  public static class Text {
    private String text = "";

    public Text() {
    }

    public Text(String text) {
      this.text = text;
    }

    public String toString() {
      return text;
    }
  }

  /** The intermediate data class. */
  public static class Count {
    private long count;

    public Count() {
    }

    public Count(long count) {
      this.count = count;
    }
  }

  /** The output class. */
  public static class WordCount {
    private String word;
    private long count;

    public WordCount() {
    }

    public WordCount(String word, long count) {
      this.word = word;
      this.count = count;
    }
  }

  public static class MapImpl extends AIngleMapper<Text, Pair<Text, Count>> {
    @Override
    public void map(Text text, AIngleCollector<Pair<Text, Count>> collector, Reporter reporter) throws IOException {
      StringTokenizer tokens = new StringTokenizer(text.toString());
      while (tokens.hasMoreTokens())
        collector.collect(new Pair<>(new Text(tokens.nextToken()), new Count(1L)));
    }
  }

  public static class ReduceImpl extends AIngleReducer<Text, Count, WordCount> {
    @Override
    public void reduce(Text word, Iterable<Count> counts, AIngleCollector<WordCount> collector, Reporter reporter)
        throws IOException {
      long sum = 0;
      for (Count count : counts)
        sum += count.count;
      collector.collect(new WordCount(word.text, sum));
    }
  }

  @Test
  @SuppressWarnings("deprecation")
  public void testJob() throws Exception {
    JobConf job = new JobConf();
    String dir = "target/testReflectJob";
    Path inputPath = new Path(dir + "/in");
    Path outputPath = new Path(dir + "/out");

    outputPath.getFileSystem(job).delete(outputPath);
    inputPath.getFileSystem(job).delete(inputPath);

    writeLinesFile(new File(dir + "/in"));

    job.setJobName("reflect");

    AIngleJob.setInputSchema(job, ReflectData.get().getSchema(Text.class));
    AIngleJob.setMapOutputSchema(job, new Pair(new Text(""), new Count(0L)).getSchema());
    AIngleJob.setOutputSchema(job, ReflectData.get().getSchema(WordCount.class));

    AIngleJob.setMapperClass(job, MapImpl.class);
    // AIngleJob.setCombinerClass(job, ReduceImpl.class);
    AIngleJob.setReducerClass(job, ReduceImpl.class);

    FileInputFormat.setInputPaths(job, inputPath);
    FileOutputFormat.setOutputPath(job, outputPath);

    AIngleJob.setReflect(job); // use reflection

    JobClient.runJob(job);

    validateCountsFile(new File(new File(dir, "out"), "part-00000.aingle"));
  }

  private void writeLinesFile(File dir) throws IOException {
    DatumWriter<Text> writer = new ReflectDatumWriter<>();
    DataFileWriter<Text> out = new DataFileWriter<>(writer);
    File linesFile = new File(dir + "/lines.aingle");
    dir.mkdirs();
    out.create(ReflectData.get().getSchema(Text.class), linesFile);
    for (String line : WordCountUtil.LINES)
      out.append(new Text(line));
    out.close();
  }

  private void validateCountsFile(File file) throws Exception {
    DatumReader<WordCount> reader = new ReflectDatumReader<>();
    InputStream in = new BufferedInputStream(new FileInputStream(file));
    DataFileStream<WordCount> counts = new DataFileStream<>(in, reader);
    int numWords = 0;
    for (WordCount wc : counts) {
      assertEquals(wc.word, WordCountUtil.COUNTS.get(wc.word), (Long) wc.count);
      numWords++;
    }
    in.close();
    assertEquals(WordCountUtil.COUNTS.size(), numWords);
  }

}
