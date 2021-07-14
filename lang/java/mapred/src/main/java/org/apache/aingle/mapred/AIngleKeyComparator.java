/*

 */

package org.apache.aingle.mapred;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.conf.Configuration;

import org.apache.aingle.Schema;
import org.apache.aingle.io.BinaryData;
import org.apache.aingle.reflect.ReflectData;

/** The {@link RawComparator} used by jobs configured with {@link AIngleJob}. */
public class AIngleKeyComparator<T> extends Configured implements RawComparator<AIngleWrapper<T>> {

  private Schema schema;

  @Override
  public void setConf(Configuration conf) {
    super.setConf(conf);
    if (conf != null)
      schema = Pair.getKeySchema(AIngleJob.getMapOutputSchema(conf));
  }

  @Override
  public int compare(byte[] b1, int s1, int l1, byte[] b2, int s2, int l2) {
    return BinaryData.compare(b1, s1, l1, b2, s2, l2, schema);
  }

  @Override
  public int compare(AIngleWrapper<T> x, AIngleWrapper<T> y) {
    return ReflectData.get().compare(x.datum(), y.datum(), schema);
  }

}
