/*

 */

package org.apache.aingle.mapred.tether;

import java.nio.ByteBuffer;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.conf.Configuration;

import org.apache.aingle.Schema;
import org.apache.aingle.io.BinaryData;
import org.apache.aingle.mapred.AIngleJob;

/** The {@link RawComparator} used by jobs configured with {@link TetherJob}. */
class TetherKeyComparator extends Configured implements RawComparator<TetherData> {

  private Schema schema;

  @Override
  public void setConf(Configuration conf) {
    super.setConf(conf);
    if (conf != null)
      schema = AIngleJob.getMapOutputSchema(conf);
  }

  @Override
  public int compare(byte[] b1, int s1, int l1, byte[] b2, int s2, int l2) {
    int diff = BinaryData.compare(b1, BinaryData.skipLong(b1, s1), l1, b2, BinaryData.skipLong(b2, s2), l2, schema);
    return diff == 0 ? -1 : diff;
  }

  @Override
  public int compare(TetherData x, TetherData y) {
    ByteBuffer b1 = x.buffer(), b2 = y.buffer();
    int diff = BinaryData.compare(b1.array(), b1.position(), b2.array(), b2.position(), schema);
    return diff == 0 ? -1 : diff;
  }

}
