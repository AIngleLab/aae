/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.RecordReader;
import org.apache.hadoop.mapred.Reporter;

/**
 * An {@link org.apache.hadoop.mapred.InputFormat} for AIngle data files, which
 * converts each datum to string form in the input key. The input value is
 * always empty. The string representation is
 * <a href="https://www.json.org/">JSON</a>.
 * <p>
 * This {@link org.apache.hadoop.mapred.InputFormat} is useful for applications
 * that wish to process AIngle data using tools like MapReduce Streaming.
 *
 * By default, when pointed at a directory, this will silently skip over any
 * files in it that do not have .aingle extension. To instead include all files,
 * set the aingle.mapred.ignore.inputs.without.extension property to false.
 */
public class AIngleAsTextInputFormat extends FileInputFormat<Text, Text> {

  @Override
  protected FileStatus[] listStatus(JobConf job) throws IOException {
    if (job.getBoolean(AIngleInputFormat.IGNORE_FILES_WITHOUT_EXTENSION_KEY,
        AIngleInputFormat.IGNORE_INPUTS_WITHOUT_EXTENSION_DEFAULT)) {
      List<FileStatus> result = new ArrayList<>();
      for (FileStatus file : super.listStatus(job))
        if (file.getPath().getName().endsWith(AIngleOutputFormat.EXT))
          result.add(file);
      return result.toArray(new FileStatus[0]);
    } else {
      return super.listStatus(job);
    }
  }

  @Override
  public RecordReader<Text, Text> getRecordReader(InputSplit split, JobConf job, Reporter reporter) throws IOException {
    reporter.setStatus(split.toString());
    return new AIngleAsTextRecordReader(job, (FileSplit) split);
  }
}
