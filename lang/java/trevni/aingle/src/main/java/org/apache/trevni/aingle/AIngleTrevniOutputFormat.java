/*

 */

package org.apache.trevni.aingle;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Map;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.RecordWriter;
import org.apache.hadoop.util.Progressable;

import org.apache.aingle.Schema;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.mapred.AIngleJob;
import org.apache.aingle.mapred.AIngleWrapper;

import org.apache.trevni.ColumnFileMetaData;

/**
 * An {@link org.apache.hadoop.mapred.OutputFormat} that writes AIngle data to
 * Trevni files.
 *
 * <p>
 * Writes a directory of files per task, each comprising a single filesystem
 * block. To reduce the number of files, increase the default filesystem block
 * size for the job. Each task also requires enough memory to buffer a
 * filesystem block.
 */
public class AIngleTrevniOutputFormat<T> extends FileOutputFormat<AIngleWrapper<T>, NullWritable> {

  /** The file name extension for trevni files. */
  public final static String EXT = ".trv";

  public static final String META_PREFIX = "trevni.meta.";

  /** Add metadata to job output files. */
  public static void setMeta(JobConf job, String key, String value) {
    job.set(META_PREFIX + key, value);
  }

  @Override
  public RecordWriter<AIngleWrapper<T>, NullWritable> getRecordWriter(FileSystem ignore, final JobConf job,
      final String name, Progressable prog) throws IOException {

    boolean isMapOnly = job.getNumReduceTasks() == 0;
    final Schema schema = isMapOnly ? AIngleJob.getMapOutputSchema(job) : AIngleJob.getOutputSchema(job);

    final ColumnFileMetaData meta = filterMetadata(job);

    final Path dir = FileOutputFormat.getTaskOutputPath(job, name);
    final FileSystem fs = dir.getFileSystem(job);
    if (!fs.mkdirs(dir))
      throw new IOException("Failed to create directory: " + dir);
    final long blockSize = fs.getDefaultBlockSize(dir);

    return new RecordWriter<AIngleWrapper<T>, NullWritable>() {
      private int part = 0;

      private AIngleColumnWriter<T> writer = new AIngleColumnWriter<>(schema, meta, ReflectData.get());

      private void flush() throws IOException {
        try (OutputStream out = fs.create(new Path(dir, "part-" + (part++) + EXT))) {
          writer.writeTo(out);
        }
        writer = new AIngleColumnWriter<>(schema, meta, ReflectData.get());
      }

      @Override
      public void write(AIngleWrapper<T> wrapper, NullWritable ignore) throws IOException {
        writer.write(wrapper.datum());
        if (writer.sizeEstimate() >= blockSize) // block full
          flush();
      }

      public void close(Reporter reporter) throws IOException {
        flush();
      }
    };
  }

  static ColumnFileMetaData filterMetadata(final JobConf job) {
    final ColumnFileMetaData meta = new ColumnFileMetaData();
    for (Map.Entry<String, String> e : job)
      if (e.getKey().startsWith(META_PREFIX))
        meta.put(e.getKey().substring(META_PREFIX.length()), e.getValue().getBytes(StandardCharsets.UTF_8));
    return meta;
  }

}
