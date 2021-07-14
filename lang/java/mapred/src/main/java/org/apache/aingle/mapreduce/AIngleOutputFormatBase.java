/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

package org.apache.aingle.mapreduce;

import java.io.IOException;
import java.io.OutputStream;
import org.apache.hadoop.mapreduce.lib.output.FileOutputCommitter;

import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.file.DataFileConstants;
import org.apache.aingle.hadoop.file.HadoopCodecFactory;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import static org.apache.aingle.file.CodecFactory.DEFAULT_ZSTANDARD_BUFFERPOOL;
import static org.apache.aingle.file.CodecFactory.DEFAULT_ZSTANDARD_LEVEL;

/**
 * Abstract base class for output formats that write AIngle container files.
 *
 * @param <K> The type of key to write.
 * @param <V> The type of value to write.
 */
public abstract class AIngleOutputFormatBase<K, V> extends FileOutputFormat<K, V> {

  /**
   * Gets the configured compression codec from the task context.
   *
   * @param context The task attempt context.
   * @return The compression codec to use for the output AIngle container file.
   */
  protected static CodecFactory getCompressionCodec(TaskAttemptContext context) {
    if (FileOutputFormat.getCompressOutput(context)) {
      // Default to deflate compression.
      int deflateLevel = context.getConfiguration().getInt(org.apache.aingle.mapred.AIngleOutputFormat.DEFLATE_LEVEL_KEY,
          CodecFactory.DEFAULT_DEFLATE_LEVEL);
      int xzLevel = context.getConfiguration().getInt(org.apache.aingle.mapred.AIngleOutputFormat.XZ_LEVEL_KEY,
          CodecFactory.DEFAULT_XZ_LEVEL);
      int zstdLevel = context.getConfiguration().getInt(org.apache.aingle.mapred.AIngleOutputFormat.ZSTD_LEVEL_KEY,
          DEFAULT_ZSTANDARD_LEVEL);
      boolean zstdBufferPool = context.getConfiguration()
          .getBoolean(org.apache.aingle.mapred.AIngleOutputFormat.ZSTD_BUFFERPOOL_KEY, DEFAULT_ZSTANDARD_BUFFERPOOL);

      String outputCodec = context.getConfiguration().get(AIngleJob.CONF_OUTPUT_CODEC);

      if (outputCodec == null) {
        String compressionCodec = context.getConfiguration().get("mapred.output.compression.codec");
        String aingleCodecName = HadoopCodecFactory.getAIngleCodecName(compressionCodec);
        if (aingleCodecName != null) {
          context.getConfiguration().set(AIngleJob.CONF_OUTPUT_CODEC, aingleCodecName);
          return HadoopCodecFactory.fromHadoopString(compressionCodec);
        } else {
          return CodecFactory.deflateCodec(deflateLevel);
        }
      } else if (DataFileConstants.DEFLATE_CODEC.equals(outputCodec)) {
        return CodecFactory.deflateCodec(deflateLevel);
      } else if (DataFileConstants.XZ_CODEC.equals(outputCodec)) {
        return CodecFactory.xzCodec(xzLevel);
      } else if (DataFileConstants.ZSTANDARD_CODEC.equals(outputCodec)) {
        return CodecFactory.zstandardCodec(zstdLevel, false, zstdBufferPool);
      } else {
        return CodecFactory.fromString(outputCodec);
      }

    }

    // No compression.
    return CodecFactory.nullCodec();
  }

  /**
   * Gets the target output stream where the AIngle container file should be
   * written.
   *
   * @param context The task attempt context.
   * @return The target output stream.
   */
  protected OutputStream getAIngleFileOutputStream(TaskAttemptContext context) throws IOException {
    Path path = new Path(((FileOutputCommitter) getOutputCommitter(context)).getWorkPath(),
        getUniqueFile(context, context.getConfiguration().get("aingle.mo.config.namedOutput", "part"),
            org.apache.aingle.mapred.AIngleOutputFormat.EXT));
    return path.getFileSystem(context.getConfiguration()).create(path);
  }

  /**
   * Gets the configured sync interval from the task context.
   *
   * @param context The task attempt context.
   * @return The sync interval to use for the output AIngle container file.
   */
  protected static int getSyncInterval(TaskAttemptContext context) {
    return context.getConfiguration().getInt(org.apache.aingle.mapred.AIngleOutputFormat.SYNC_INTERVAL_KEY,
        DataFileConstants.DEFAULT_SYNC_INTERVAL);
  }

}
