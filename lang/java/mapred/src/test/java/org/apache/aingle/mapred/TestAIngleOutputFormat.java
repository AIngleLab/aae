/*

 */
package org.apache.aingle.mapred;

import org.apache.aingle.file.CodecFactory;
import org.apache.hadoop.mapred.JobConf;
import org.junit.Test;

import static org.junit.Assert.*;

public class TestAIngleOutputFormat {
  @Test
  public void testSetSyncInterval() {
    JobConf jobConf = new JobConf();
    int newSyncInterval = 100000;
    AIngleOutputFormat.setSyncInterval(jobConf, newSyncInterval);

    assertEquals(newSyncInterval, jobConf.getInt(AIngleOutputFormat.SYNC_INTERVAL_KEY, -1));
  }

  @Test
  public void testNoCodec() {
    JobConf job = new JobConf();
    assertNull(AIngleOutputFormat.getCodecFactory(job));

    job = new JobConf();
    job.set("mapred.output.compress", "false");
    job.set("mapred.output.compression.codec", "org.apache.hadoop.io.compress.BZip2Codec");
    assertNull(AIngleOutputFormat.getCodecFactory(job));

    job = new JobConf();
    job.set("mapred.output.compress", "false");
    job.set(AIngleJob.OUTPUT_CODEC, "bzip2");
    assertNull(AIngleOutputFormat.getCodecFactory(job));
  }

  @Test
  public void testBZip2CodecUsingHadoopClass() {
    CodecFactory aingleBZip2Codec = CodecFactory.fromString("bzip2");

    JobConf job = new JobConf();
    job.set("mapred.output.compress", "true");
    job.set("mapred.output.compression.codec", "org.apache.hadoop.io.compress.BZip2Codec");
    CodecFactory factory = AIngleOutputFormat.getCodecFactory(job);
    assertNotNull(factory);
    assertEquals(factory.getClass(), aingleBZip2Codec.getClass());
  }

  @Test
  public void testBZip2CodecUsingAIngleCodec() {
    CodecFactory aingleBZip2Codec = CodecFactory.fromString("bzip2");

    JobConf job = new JobConf();
    job.set("mapred.output.compress", "true");
    job.set(AIngleJob.OUTPUT_CODEC, "bzip2");
    CodecFactory factory = AIngleOutputFormat.getCodecFactory(job);
    assertNotNull(factory);
    assertEquals(factory.getClass(), aingleBZip2Codec.getClass());
  }

  @Test
  public void testDeflateCodecUsingHadoopClass() {
    CodecFactory aingleDeflateCodec = CodecFactory.fromString("deflate");

    JobConf job = new JobConf();
    job.set("mapred.output.compress", "true");
    job.set("mapred.output.compression.codec", "org.apache.hadoop.io.compress.DeflateCodec");
    CodecFactory factory = AIngleOutputFormat.getCodecFactory(job);
    assertNotNull(factory);
    assertEquals(factory.getClass(), aingleDeflateCodec.getClass());
  }

  @Test
  public void testDeflateCodecUsingAIngleCodec() {
    CodecFactory aingleDeflateCodec = CodecFactory.fromString("deflate");

    JobConf job = new JobConf();
    job.set("mapred.output.compress", "true");
    job.set(AIngleJob.OUTPUT_CODEC, "deflate");
    CodecFactory factory = AIngleOutputFormat.getCodecFactory(job);
    assertNotNull(factory);
    assertEquals(factory.getClass(), aingleDeflateCodec.getClass());
  }

  @Test
  public void testSnappyCodecUsingHadoopClass() {
    CodecFactory aingleSnappyCodec = CodecFactory.fromString("snappy");

    JobConf job = new JobConf();
    job.set("mapred.output.compress", "true");
    job.set("mapred.output.compression.codec", "org.apache.hadoop.io.compress.SnappyCodec");
    CodecFactory factory = AIngleOutputFormat.getCodecFactory(job);
    assertNotNull(factory);
    assertEquals(factory.getClass(), aingleSnappyCodec.getClass());
  }

  @Test
  public void testSnappyCodecUsingAIngleCodec() {
    CodecFactory aingleSnappyCodec = CodecFactory.fromString("snappy");

    JobConf job = new JobConf();
    job.set("mapred.output.compress", "true");
    job.set(AIngleJob.OUTPUT_CODEC, "snappy");
    CodecFactory factory = AIngleOutputFormat.getCodecFactory(job);
    assertNotNull(factory);
    assertEquals(factory.getClass(), aingleSnappyCodec.getClass());
  }

  @Test
  public void testGZipCodecUsingHadoopClass() {
    CodecFactory aingleDeflateCodec = CodecFactory.fromString("deflate");

    JobConf job = new JobConf();
    job.set("mapred.output.compress", "true");
    job.set("mapred.output.compression.codec", "org.apache.hadoop.io.compress.GZipCodec");
    CodecFactory factory = AIngleOutputFormat.getCodecFactory(job);
    assertNotNull(factory);
    assertEquals(factory.getClass(), aingleDeflateCodec.getClass());
  }
}
