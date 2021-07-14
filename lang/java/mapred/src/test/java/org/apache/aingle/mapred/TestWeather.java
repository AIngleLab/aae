/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.io.File;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.Reporter;

import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.file.DataFileReader;
import static org.apache.aingle.file.DataFileConstants.SNAPPY_CODEC;
import static org.junit.Assert.assertEquals;

import org.junit.After;
import org.junit.Test;

import test.Weather;

/** Tests mapred API with a specific record. */
public class TestWeather {

  private static final AtomicInteger mapCloseCalls = new AtomicInteger();
  private static final AtomicInteger mapConfigureCalls = new AtomicInteger();
  private static final AtomicInteger reducerCloseCalls = new AtomicInteger();
  private static final AtomicInteger reducerConfigureCalls = new AtomicInteger();

  @After
  public void tearDown() {
    mapCloseCalls.set(0);
    mapConfigureCalls.set(0);
    reducerCloseCalls.set(0);
    reducerConfigureCalls.set(0);
  }

  /** Uses default mapper with no reduces for a map-only identity job. */
  @Test
  @SuppressWarnings("deprecation")
  public void testMapOnly() throws Exception {
    JobConf job = new JobConf();
    String inDir = System.getProperty("share.dir", "../../../share") + "/test/data";
    Path input = new Path(inDir + "/weather.aingle");
    Path output = new Path("target/test/weather-ident");

    output.getFileSystem(job).delete(output);

    job.setJobName("identity map weather");

    AIngleJob.setInputSchema(job, Weather.SCHEMA$);
    AIngleJob.setOutputSchema(job, Weather.SCHEMA$);

    FileInputFormat.setInputPaths(job, input);
    FileOutputFormat.setOutputPath(job, output);
    FileOutputFormat.setCompressOutput(job, true);

    job.setNumReduceTasks(0); // map-only

    JobClient.runJob(job);

    // check output is correct
    DatumReader<Weather> reader = new SpecificDatumReader<>();
    DataFileReader<Weather> check = new DataFileReader<>(new File(inDir + "/weather.aingle"), reader);
    DataFileReader<Weather> sorted = new DataFileReader<>(new File(output.toString() + "/part-00000.aingle"), reader);

    for (Weather w : sorted)
      assertEquals(check.next(), w);

    check.close();
    sorted.close();
  }

  // maps input Weather to Pair<Weather,Void>, to sort by Weather
  public static class SortMapper extends AIngleMapper<Weather, Pair<Weather, Void>> {
    @Override
    public void map(Weather w, AIngleCollector<Pair<Weather, Void>> collector, Reporter reporter) throws IOException {
      collector.collect(new Pair<>(w, (Void) null));
    }

    @Override
    public void close() throws IOException {
      mapCloseCalls.incrementAndGet();
    }

    @Override
    public void configure(JobConf jobConf) {
      mapConfigureCalls.incrementAndGet();
    }
  }

  // output keys only, since values are empty
  public static class SortReducer extends AIngleReducer<Weather, Void, Weather> {
    @Override
    public void reduce(Weather w, Iterable<Void> ignore, AIngleCollector<Weather> collector, Reporter reporter)
        throws IOException {
      collector.collect(w);
    }

    @Override
    public void close() throws IOException {
      reducerCloseCalls.incrementAndGet();
    }

    @Override
    public void configure(JobConf jobConf) {
      reducerConfigureCalls.incrementAndGet();
    }
  }

  @Test
  @SuppressWarnings("deprecation")
  public void testSort() throws Exception {
    JobConf job = new JobConf();
    String inDir = "../../../share/test/data";
    Path input = new Path(inDir + "/weather.aingle");
    Path output = new Path("target/test/weather-sort");

    output.getFileSystem(job).delete(output);

    job.setJobName("sort weather");

    AIngleJob.setInputSchema(job, Weather.SCHEMA$);
    AIngleJob.setMapOutputSchema(job, Pair.getPairSchema(Weather.SCHEMA$, Schema.create(Type.NULL)));
    AIngleJob.setOutputSchema(job, Weather.SCHEMA$);

    AIngleJob.setMapperClass(job, SortMapper.class);
    AIngleJob.setReducerClass(job, SortReducer.class);

    FileInputFormat.setInputPaths(job, input);
    FileOutputFormat.setOutputPath(job, output);
    FileOutputFormat.setCompressOutput(job, true);
    AIngleJob.setOutputCodec(job, SNAPPY_CODEC);

    JobClient.runJob(job);

    // check output is correct
    DatumReader<Weather> reader = new SpecificDatumReader<>();
    DataFileReader<Weather> check = new DataFileReader<>(new File(inDir + "/weather-sorted.aingle"), reader);
    DataFileReader<Weather> sorted = new DataFileReader<>(new File(output.toString() + "/part-00000.aingle"), reader);

    for (Weather w : sorted)
      assertEquals(check.next(), w);

    check.close();
    sorted.close();

    // check that AIngleMapper and AIngleReducer get close() and configure() called
    assertEquals(1, mapCloseCalls.get());
    assertEquals(1, reducerCloseCalls.get());
    assertEquals(1, mapConfigureCalls.get());
    assertEquals(1, reducerConfigureCalls.get());

  }

}
