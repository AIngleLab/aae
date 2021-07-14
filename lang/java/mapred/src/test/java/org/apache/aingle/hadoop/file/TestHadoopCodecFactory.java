/*

 */
package org.apache.aingle.hadoop.file;

import org.apache.aingle.file.CodecFactory;
import org.junit.Test;

import static org.junit.Assert.assertTrue;

public class TestHadoopCodecFactory {

  @Test
  public void testHadoopCodecFactoryDeflate() {
    CodecFactory hadoopDeflateCodec = HadoopCodecFactory.fromHadoopString("org.apache.hadoop.io.compress.DeflateCodec");
    CodecFactory aingleDeflateCodec = CodecFactory.fromString("deflate");
    assertTrue(hadoopDeflateCodec.getClass().equals(aingleDeflateCodec.getClass()));
  }

  @Test
  public void testHadoopCodecFactorySnappy() {
    CodecFactory hadoopSnappyCodec = HadoopCodecFactory.fromHadoopString("org.apache.hadoop.io.compress.SnappyCodec");
    CodecFactory aingleSnappyCodec = CodecFactory.fromString("snappy");
    assertTrue(hadoopSnappyCodec.getClass().equals(aingleSnappyCodec.getClass()));
  }

  @Test
  public void testHadoopCodecFactoryBZip2() {
    CodecFactory hadoopSnappyCodec = HadoopCodecFactory.fromHadoopString("org.apache.hadoop.io.compress.BZip2Codec");
    CodecFactory aingleSnappyCodec = CodecFactory.fromString("bzip2");
    assertTrue(hadoopSnappyCodec.getClass().equals(aingleSnappyCodec.getClass()));
  }

  @Test
  public void testHadoopCodecFactoryGZip() {
    CodecFactory hadoopSnappyCodec = HadoopCodecFactory.fromHadoopString("org.apache.hadoop.io.compress.GZipCodec");
    CodecFactory aingleSnappyCodec = CodecFactory.fromString("deflate");
    assertTrue(hadoopSnappyCodec.getClass().equals(aingleSnappyCodec.getClass()));
  }

  @Test
  public void testHadoopCodecFactoryFail() {
    CodecFactory hadoopSnappyCodec = HadoopCodecFactory.fromHadoopString("org.apache.hadoop.io.compress.FooCodec");
    assertTrue(hadoopSnappyCodec == null);
  }
}
