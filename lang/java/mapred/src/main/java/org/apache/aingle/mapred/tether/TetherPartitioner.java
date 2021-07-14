/*

 */

package org.apache.aingle.mapred.tether;

import java.nio.ByteBuffer;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.Partitioner;

import org.apache.aingle.Schema;
import org.apache.aingle.io.BinaryData;
import org.apache.aingle.mapred.AIngleJob;

class TetherPartitioner implements Partitioner<TetherData, NullWritable> {

  private static final ThreadLocal<Integer> CACHE = new ThreadLocal<>();

  private Schema schema;

  @Override
  public void configure(JobConf job) {
    schema = AIngleJob.getMapOutputSchema(job);
  }

  static void setNextPartition(int newValue) {
    CACHE.set(newValue);
  }

  @Override
  public int getPartition(TetherData key, NullWritable value, int numPartitions) {
    Integer result = CACHE.get();
    if (result != null) // return cached value
      return result;

    ByteBuffer b = key.buffer();
    int p = b.position();
    int hashCode = BinaryData.hashCode(b.array(), p, b.limit() - p, schema);
    if (hashCode < 0)
      hashCode = -hashCode;
    return hashCode % numPartitions;
  }

}
