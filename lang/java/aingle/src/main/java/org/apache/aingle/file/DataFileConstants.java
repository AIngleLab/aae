/*

 */

package org.apache.aingle.file;

/**
 * Constants used in data files.
 */
public class DataFileConstants {
  private DataFileConstants() {
  } // no public ctor

  public static final byte VERSION = 1;
  public static final byte[] MAGIC = new byte[] { (byte) 'O', (byte) 'b', (byte) 'j', VERSION };
  public static final long FOOTER_BLOCK = -1;
  public static final int SYNC_SIZE = 16;
  public static final int DEFAULT_SYNC_INTERVAL = 4000 * SYNC_SIZE;

  public static final String SCHEMA = "aingle.schema";
  public static final String CODEC = "aingle.codec";
  public static final String NULL_CODEC = "null";
  public static final String DEFLATE_CODEC = "deflate";
  public static final String SNAPPY_CODEC = "snappy";
  public static final String BZIP2_CODEC = "bzip2";
  public static final String XZ_CODEC = "xz";
  public static final String ZSTANDARD_CODEC = "zstandard";

}
