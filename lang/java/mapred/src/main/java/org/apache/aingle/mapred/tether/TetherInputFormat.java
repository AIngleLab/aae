/*

 */

package org.apache.aingle.mapred.tether;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.RecordReader;

import org.apache.aingle.mapred.AIngleInputFormat;
import org.apache.aingle.mapred.AIngleOutputFormat;

/**
 * An {@link org.apache.hadoop.mapred.InputFormat} for tethered AIngle input.
 *
 * By default, when pointed at a directory, this will silently skip over any
 * files in it that do not have .aingle extension. To instead include all files,
 * set the aingle.mapred.ignore.inputs.without.extension property to false.
 */
class TetherInputFormat extends FileInputFormat<TetherData, NullWritable> {

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
  public RecordReader<TetherData, NullWritable> getRecordReader(InputSplit split, JobConf job, Reporter reporter)
      throws IOException {
    reporter.setStatus(split.toString());
    return new TetherRecordReader(job, (FileSplit) split);
  }

}
