/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Map;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.RecordWriter;
import org.apache.hadoop.util.Progressable;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.hadoop.file.HadoopCodecFactory;

import static org.apache.aingle.file.DataFileConstants.DEFAULT_SYNC_INTERVAL;
import static org.apache.aingle.file.DataFileConstants.DEFLATE_CODEC;
import static org.apache.aingle.file.DataFileConstants.XZ_CODEC;
import static org.apache.aingle.file.DataFileConstants.ZSTANDARD_CODEC;
import static org.apache.aingle.file.CodecFactory.DEFAULT_DEFLATE_LEVEL;
import static org.apache.aingle.file.CodecFactory.DEFAULT_XZ_LEVEL;
import static org.apache.aingle.file.CodecFactory.DEFAULT_ZSTANDARD_LEVEL;
import static org.apache.aingle.file.CodecFactory.DEFAULT_ZSTANDARD_BUFFERPOOL;

/**
 * An {@link org.apache.hadoop.mapred.OutputFormat} for AIngle data files.
 * <p/>
 * You can specify various options using Job Configuration properties. Look at
 * the fields in {@link AIngleJob} as well as this class to get an overview of the
 * supported options.
 */
public class AIngleOutputFormat<T> extends FileOutputFormat<AIngleWrapper<T>, NullWritable> {

  /** The file name extension for aingle data files. */
  public final static String EXT = ".aingle";

  /** The configuration key for AIngle deflate level. */
  public static final String DEFLATE_LEVEL_KEY = "aingle.mapred.deflate.level";

  /** The configuration key for AIngle XZ level. */
  public static final String XZ_LEVEL_KEY = "aingle.mapred.xz.level";

  /** The configuration key for AIngle ZSTD level. */
  public static final String ZSTD_LEVEL_KEY = "aingle.mapred.zstd.level";

  /** The configuration key for AIngle ZSTD buffer pool. */
  public static final String ZSTD_BUFFERPOOL_KEY = "aingle.mapred.zstd.bufferpool";

  /** The configuration key for AIngle sync interval. */
  public static final String SYNC_INTERVAL_KEY = "aingle.mapred.sync.interval";

  /** Enable output compression using the deflate codec and specify its level. */
  public static void setDeflateLevel(JobConf job, int level) {
    FileOutputFormat.setCompressOutput(job, true);
    job.setInt(DEFLATE_LEVEL_KEY, level);
  }

  /**
   * Set the sync interval to be used by the underlying {@link DataFileWriter}.
   */
  public static void setSyncInterval(JobConf job, int syncIntervalInBytes) {
    job.setInt(SYNC_INTERVAL_KEY, syncIntervalInBytes);
  }

  static <T> void configureDataFileWriter(DataFileWriter<T> writer, JobConf job) throws UnsupportedEncodingException {

    CodecFactory factory = getCodecFactory(job);

    if (factory != null) {
      writer.setCodec(factory);
    }

    writer.setSyncInterval(job.getInt(SYNC_INTERVAL_KEY, DEFAULT_SYNC_INTERVAL));

    // copy metadata from job
    for (Map.Entry<String, String> e : job) {
      if (e.getKey().startsWith(AIngleJob.TEXT_PREFIX))
        writer.setMeta(e.getKey().substring(AIngleJob.TEXT_PREFIX.length()), e.getValue());
      if (e.getKey().startsWith(AIngleJob.BINARY_PREFIX))
        writer.setMeta(e.getKey().substring(AIngleJob.BINARY_PREFIX.length()),
            URLDecoder.decode(e.getValue(), StandardCharsets.ISO_8859_1.name()).getBytes(StandardCharsets.ISO_8859_1));
    }
  }

  /**
   * This will select the correct compression codec from the JobConf. The order of
   * selection is as follows:
   * <ul>
   * <li>If mapred.output.compress is true then look for codec otherwise no
   * compression</li>
   * <li>Use aingle.output.codec if populated</li>
   * <li>Next use mapred.output.compression.codec if populated</li>
   * <li>If not default to Deflate Codec</li>
   * </ul>
   */
  static CodecFactory getCodecFactory(JobConf job) {
    CodecFactory factory = null;

    if (FileOutputFormat.getCompressOutput(job)) {
      int deflateLevel = job.getInt(DEFLATE_LEVEL_KEY, DEFAULT_DEFLATE_LEVEL);
      int xzLevel = job.getInt(XZ_LEVEL_KEY, DEFAULT_XZ_LEVEL);
      int zstdLevel = job.getInt(ZSTD_LEVEL_KEY, DEFAULT_ZSTANDARD_LEVEL);
      boolean zstdBufferPool = job.getBoolean(ZSTD_BUFFERPOOL_KEY, DEFAULT_ZSTANDARD_BUFFERPOOL);
      String codecName = job.get(AIngleJob.OUTPUT_CODEC);

      if (codecName == null) {
        String codecClassName = job.get("mapred.output.compression.codec", null);
        String aingleCodecName = HadoopCodecFactory.getAIngleCodecName(codecClassName);
        if (codecClassName != null && aingleCodecName != null) {
          factory = HadoopCodecFactory.fromHadoopString(codecClassName);
          job.set(AIngleJob.OUTPUT_CODEC, aingleCodecName);
          return factory;
        } else {
          return CodecFactory.deflateCodec(deflateLevel);
        }
      } else {
        if (codecName.equals(DEFLATE_CODEC)) {
          factory = CodecFactory.deflateCodec(deflateLevel);
        } else if (codecName.equals(XZ_CODEC)) {
          factory = CodecFactory.xzCodec(xzLevel);
        } else if (codecName.equals(ZSTANDARD_CODEC)) {
          factory = CodecFactory.zstandardCodec(zstdLevel, false, zstdBufferPool);
        } else {
          factory = CodecFactory.fromString(codecName);
        }
      }
    }

    return factory;
  }

  @Override
  public RecordWriter<AIngleWrapper<T>, NullWritable> getRecordWriter(FileSystem ignore, JobConf job, String name,
      Progressable prog) throws IOException {

    boolean isMapOnly = job.getNumReduceTasks() == 0;
    Schema schema = isMapOnly ? AIngleJob.getMapOutputSchema(job) : AIngleJob.getOutputSchema(job);
    GenericData dataModel = AIngleJob.createDataModel(job);

    final DataFileWriter<T> writer = new DataFileWriter<T>(dataModel.createDatumWriter(null));

    configureDataFileWriter(writer, job);

    Path path = FileOutputFormat.getTaskOutputPath(job, name + EXT);
    writer.create(schema, path.getFileSystem(job).create(path));

    return new RecordWriter<AIngleWrapper<T>, NullWritable>() {
      @Override
      public void write(AIngleWrapper<T> wrapper, NullWritable ignore) throws IOException {
        writer.append(wrapper.datum());
      }

      @Override
      public void close(Reporter reporter) throws IOException {
        writer.close();
      }
    };
  }

}
