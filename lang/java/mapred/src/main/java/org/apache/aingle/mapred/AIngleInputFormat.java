/*

 */

package org.apache.aingle.mapred;

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

/**
 * An {@link org.apache.hadoop.mapred.InputFormat} for AIngle data files.
 *
 * By default, when pointed at a directory, this will silently skip over any
 * files in it that do not have .aingle extension. To instead include all files,
 * set the aingle.mapred.ignore.inputs.without.extension property to false.
 */
public class AIngleInputFormat<T> extends FileInputFormat<AIngleWrapper<T>, NullWritable> {

  /** Whether to silently ignore input files without the .aingle extension */
  public static final String IGNORE_FILES_WITHOUT_EXTENSION_KEY = "aingle.mapred.ignore.inputs.without.extension";

  /**
   * Default of whether to silently ignore input files without the .aingle
   * extension.
   */
  public static final boolean IGNORE_INPUTS_WITHOUT_EXTENSION_DEFAULT = true;

  @Override
  protected FileStatus[] listStatus(JobConf job) throws IOException {
    FileStatus[] status = super.listStatus(job);
    if (job.getBoolean(IGNORE_FILES_WITHOUT_EXTENSION_KEY, IGNORE_INPUTS_WITHOUT_EXTENSION_DEFAULT)) {
      List<FileStatus> result = new ArrayList<>(status.length);
      for (FileStatus file : status)
        if (file.getPath().getName().endsWith(AIngleOutputFormat.EXT))
          result.add(file);
      status = result.toArray(new FileStatus[0]);
    }
    return status;
  }

  @Override
  public RecordReader<AIngleWrapper<T>, NullWritable> getRecordReader(InputSplit split, JobConf job, Reporter reporter)
      throws IOException {
    reporter.setStatus(split.toString());
    return new AIngleRecordReader<>(job, (FileSplit) split);
  }

}
