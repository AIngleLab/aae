/*

 */
package org.apache.trevni.aingle;

import org.apache.aingle.mapred.AIngleJob;
import org.apache.hadoop.mapred.JobConf;
import org.apache.trevni.ColumnFileMetaData;
import org.junit.Test;
import static org.junit.Assert.*;

public class TestMetadataFiltering {

  @Test
  public void testMetadataFiltering() throws Exception {
    JobConf job = new JobConf();

    job.set(AIngleTrevniOutputFormat.META_PREFIX + "test1", "1");
    job.set(AIngleTrevniOutputFormat.META_PREFIX + "test2", "2");
    job.set("test3", "3");
    job.set(AIngleJob.TEXT_PREFIX + "test4", "4");
    job.set(AIngleTrevniOutputFormat.META_PREFIX + "test5", "5");

    ColumnFileMetaData metadata = AIngleTrevniOutputFormat.filterMetadata(job);

    assertTrue(metadata.get("test1") != null);
    assertTrue(new String(metadata.get("test1")).equals("1"));
    assertTrue(metadata.get("test2") != null);
    assertTrue(new String(metadata.get("test2")).equals("2"));
    assertTrue(metadata.get("test5") != null);
    assertTrue(new String(metadata.get("test5")).equals("5"));
    assertTrue(metadata.get("test3") == null);
    assertTrue(metadata.get("test4") == null);

  }

}
