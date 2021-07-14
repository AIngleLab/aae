/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.util.ReflectionUtils;

/**
 * Bridge between a {@link org.apache.hadoop.mapred.Reducer} and an
 * {@link AIngleReducer}.
 */
class HadoopReducer<K, V, OUT> extends HadoopReducerBase<K, V, OUT, AIngleWrapper<OUT>, NullWritable> {

  @Override
  @SuppressWarnings("unchecked")
  protected AIngleReducer<K, V, OUT> getReducer(JobConf conf) {
    return ReflectionUtils.newInstance(conf.getClass(AIngleJob.REDUCER, AIngleReducer.class, AIngleReducer.class), conf);
  }

  private class ReduceCollector extends AIngleCollector<OUT> {
    private final AIngleWrapper<OUT> wrapper = new AIngleWrapper<>(null);
    private OutputCollector<AIngleWrapper<OUT>, NullWritable> out;

    public ReduceCollector(OutputCollector<AIngleWrapper<OUT>, NullWritable> out) {
      this.out = out;
    }

    @Override
    public void collect(OUT datum) throws IOException {
      wrapper.datum(datum);
      out.collect(wrapper, NullWritable.get());
    }
  }

  @Override
  protected AIngleCollector<OUT> getCollector(OutputCollector<AIngleWrapper<OUT>, NullWritable> collector) {
    return new ReduceCollector(collector);
  }

}
