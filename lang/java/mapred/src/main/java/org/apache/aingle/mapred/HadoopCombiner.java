/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.util.ReflectionUtils;

/**
 * Bridge between a {@link org.apache.hadoop.mapred.Reducer} and an
 * {@link AIngleReducer} used when combining. When combining, map output pairs
 * must be split before they're collected.
 */
class HadoopCombiner<K, V> extends HadoopReducerBase<K, V, Pair<K, V>, AIngleKey<K>, AIngleValue<V>> {

  @Override
  @SuppressWarnings("unchecked")
  protected AIngleReducer<K, V, Pair<K, V>> getReducer(JobConf conf) {
    return ReflectionUtils.newInstance(conf.getClass(AIngleJob.COMBINER, AIngleReducer.class, AIngleReducer.class), conf);
  }

  private class PairCollector extends AIngleCollector<Pair<K, V>> {
    private final AIngleKey<K> keyWrapper = new AIngleKey<>(null);
    private final AIngleValue<V> valueWrapper = new AIngleValue<>(null);
    private OutputCollector<AIngleKey<K>, AIngleValue<V>> collector;

    public PairCollector(OutputCollector<AIngleKey<K>, AIngleValue<V>> collector) {
      this.collector = collector;
    }

    @Override
    public void collect(Pair<K, V> datum) throws IOException {
      keyWrapper.datum(datum.key()); // split the Pair
      valueWrapper.datum(datum.value());
      collector.collect(keyWrapper, valueWrapper);
    }
  }

  @Override
  protected AIngleCollector<Pair<K, V>> getCollector(OutputCollector<AIngleKey<K>, AIngleValue<V>> collector) {
    return new PairCollector(collector);
  }

}
