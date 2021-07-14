/*

 */
package org.apache.aingle.hadoop.file;

import java.util.HashMap;
import java.util.Map;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.file.CodecFactory;

/**
 * Encapsulates the ability to specify and configure an aingle compression codec
 * from a given hadoop codec defined with the configuration parameter:
 * mapred.output.compression.codec
 *
 * Currently there are three codecs registered by default:
 * <ul>
 * <li>{@code org.apache.hadoop.io.compress.DeflateCodec} will map to
 * {@code deflate}</li>
 * <li>{@code org.apache.hadoop.io.compress.SnappyCodec} will map to
 * {@code snappy}</li>
 * <li>{@code org.apache.hadoop.io.compress.BZip2Codec} will map to
 * {@code zbip2}</li>
 * <li>{@code org.apache.hadoop.io.compress.GZipCodec} will map to
 * {@code deflate}</li>
 * </ul>
 */
public class HadoopCodecFactory {

  private static final Map<String, String> HADOOP_AINGLE_NAME_MAP = new HashMap<>();

  static {
    HADOOP_AINGLE_NAME_MAP.put("org.apache.hadoop.io.compress.DeflateCodec", "deflate");
    HADOOP_AINGLE_NAME_MAP.put("org.apache.hadoop.io.compress.SnappyCodec", "snappy");
    HADOOP_AINGLE_NAME_MAP.put("org.apache.hadoop.io.compress.BZip2Codec", "bzip2");
    HADOOP_AINGLE_NAME_MAP.put("org.apache.hadoop.io.compress.GZipCodec", "deflate");
  }

  /**
   * Maps a hadoop codec name into a CodecFactory.
   *
   * Currently there are four hadoop codecs registered:
   * <ul>
   * <li>{@code org.apache.hadoop.io.compress.DeflateCodec} will map to
   * {@code deflate}</li>
   * <li>{@code org.apache.hadoop.io.compress.SnappyCodec} will map to
   * {@code snappy}</li>
   * <li>{@code org.apache.hadoop.io.compress.BZip2Codec} will map to
   * {@code zbip2}</li>
   * <li>{@code org.apache.hadoop.io.compress.GZipCodec} will map to
   * {@code deflate}</li>
   * </ul>
   */
  public static CodecFactory fromHadoopString(String hadoopCodecClass) {

    CodecFactory o = null;
    try {
      String aingleCodec = HADOOP_AINGLE_NAME_MAP.get(hadoopCodecClass);
      if (aingleCodec != null) {
        o = CodecFactory.fromString(aingleCodec);
      }
    } catch (Exception e) {
      throw new AIngleRuntimeException("Unrecognized hadoop codec: " + hadoopCodecClass, e);
    }
    return o;
  }

  public static String getAIngleCodecName(String hadoopCodecClass) {
    return HADOOP_AINGLE_NAME_MAP.get(hadoopCodecClass);
  }
}
