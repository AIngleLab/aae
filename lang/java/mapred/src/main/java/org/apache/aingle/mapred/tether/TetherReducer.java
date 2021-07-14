/*

 */

package org.apache.aingle.mapred.tether;

import java.io.IOException;
import java.util.Iterator;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;

import org.apache.aingle.mapred.AIngleJob;

class TetherReducer implements Reducer<TetherData, NullWritable, TetherData, NullWritable> {

  private JobConf job;
  private TetheredProcess process;
  private boolean error;

  @Override
  public void configure(JobConf job) {
    this.job = job;
  }

  @Override
  public void reduce(TetherData datum, Iterator<NullWritable> ignore,
      OutputCollector<TetherData, NullWritable> collector, Reporter reporter) throws IOException {
    try {
      if (process == null) {
        process = new TetheredProcess(job, collector, reporter);
        process.inputClient.configure(TaskType.REDUCE, AIngleJob.getMapOutputSchema(job).toString(),
            AIngleJob.getOutputSchema(job).toString());
      }
      process.inputClient.input(datum.buffer(), datum.count());
    } catch (IOException e) {
      error = true;
      throw e;
    } catch (Exception e) {
      error = true;
      throw new IOException(e);
    }
  }

  /**
   * Handle the end of the input by closing down the application.
   */
  @Override
  public void close() throws IOException {
    if (process == null)
      return;
    try {
      if (error)
        process.inputClient.abort();
      else
        process.inputClient.complete();
      process.outputService.waitForFinish();
    } catch (InterruptedException e) {
      throw new IOException(e);
    } finally {
      process.close();
    }
  }
}
