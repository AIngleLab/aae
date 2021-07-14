/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.OutputCollector;

@SuppressWarnings("unchecked")
class MapCollector<OUT, K, V, KO, VO> extends AIngleCollector<OUT> {
  private final AIngleWrapper<OUT> wrapper = new AIngleWrapper<>(null);
  private final AIngleKey<K> keyWrapper = new AIngleKey<>(null);
  private final AIngleValue<V> valueWrapper = new AIngleValue<>(null);
  private OutputCollector<KO, VO> collector;
  private boolean isMapOnly;

  public MapCollector(OutputCollector<KO, VO> collector, boolean isMapOnly) {
    this.collector = collector;
    this.isMapOnly = isMapOnly;
  }

  @Override
  public void collect(OUT datum) throws IOException {
    if (isMapOnly) {
      wrapper.datum(datum);
      collector.collect((KO) wrapper, (VO) NullWritable.get());
    } else {
      // split a pair
      Pair<K, V> pair = (Pair<K, V>) datum;
      keyWrapper.datum(pair.key());
      valueWrapper.datum(pair.value());
      collector.collect((KO) keyWrapper, (VO) valueWrapper);
    }
  }
}
