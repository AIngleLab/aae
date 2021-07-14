/*

 */

package org.apache.aingle.mapred.tether;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.RecordWriter;
import org.apache.hadoop.util.Progressable;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.mapred.AIngleJob;
import org.apache.aingle.mapred.AIngleOutputFormat;

/** An {@link org.apache.hadoop.mapred.OutputFormat} for AIngle data files. */
class TetherOutputFormat extends FileOutputFormat<TetherData, NullWritable> {

  /** Enable output compression using the deflate codec and specify its level. */
  public static void setDeflateLevel(JobConf job, int level) {
    FileOutputFormat.setCompressOutput(job, true);
    job.setInt(AIngleOutputFormat.DEFLATE_LEVEL_KEY, level);
  }

  @SuppressWarnings("unchecked")
  @Override
  public RecordWriter<TetherData, NullWritable> getRecordWriter(FileSystem ignore, JobConf job, String name,
      Progressable prog) throws IOException {

    Schema schema = AIngleJob.getOutputSchema(job);

    final DataFileWriter writer = new DataFileWriter(new GenericDatumWriter());

    if (FileOutputFormat.getCompressOutput(job)) {
      int level = job.getInt(AIngleOutputFormat.DEFLATE_LEVEL_KEY, CodecFactory.DEFAULT_DEFLATE_LEVEL);
      writer.setCodec(CodecFactory.deflateCodec(level));
    }

    Path path = FileOutputFormat.getTaskOutputPath(job, name + AIngleOutputFormat.EXT);
    writer.create(schema, path.getFileSystem(job).create(path));

    return new RecordWriter<TetherData, NullWritable>() {
      @Override
      public void write(TetherData datum, NullWritable ignore) throws IOException {
        writer.appendEncoded(datum.buffer());
      }

      @Override
      public void close(Reporter reporter) throws IOException {
        writer.close();
      }
    };
  }

}
