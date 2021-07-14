/*

 */

package org.apache.aingle.mapred;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestAIngleTextSort {

  @Rule
  public TemporaryFolder INPUT_DIR = new TemporaryFolder();

  @Rule
  public TemporaryFolder OUTPUT_DIR = new TemporaryFolder();

  @Test
  /**
   * Run the identity job on a "bytes" AIngle file using AIngleAsTextInputFormat and
   * AIngleTextOutputFormat to produce a sorted "bytes" AIngle file.
   */
  public void testSort() throws Exception {
    JobConf job = new JobConf();
    String inputPath = INPUT_DIR.getRoot().getPath();
    Path outputPath = new Path(OUTPUT_DIR.getRoot().getPath());
    outputPath.getFileSystem(job).delete(outputPath, true);

    WordCountUtil.writeLinesBytesFile(inputPath);

    job.setInputFormat(AIngleAsTextInputFormat.class);
    job.setOutputFormat(AIngleTextOutputFormat.class);
    job.setOutputKeyClass(Text.class);

    FileInputFormat.setInputPaths(job, new Path(inputPath));
    FileOutputFormat.setOutputPath(job, outputPath);

    JobClient.runJob(job);

    WordCountUtil.validateSortedFile(outputPath.toString() + "/part-00000.aingle");
  }

}
