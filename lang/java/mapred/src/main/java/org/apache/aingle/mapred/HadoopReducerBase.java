/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.util.Iterator;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.Reducer;

abstract class HadoopReducerBase<K, V, OUT, KO, VO> extends MapReduceBase
    implements Reducer<AIngleKey<K>, AIngleValue<V>, KO, VO> {

  private AIngleReducer<K, V, OUT> reducer;
  private AIngleCollector<OUT> collector;

  protected abstract AIngleReducer<K, V, OUT> getReducer(JobConf conf);

  protected abstract AIngleCollector<OUT> getCollector(OutputCollector<KO, VO> c);

  @Override
  public void configure(JobConf conf) {
    this.reducer = getReducer(conf);
  }

  class ReduceIterable implements Iterable<V>, Iterator<V> {
    private Iterator<AIngleValue<V>> values;

    @Override
    public boolean hasNext() {
      return values.hasNext();
    }

    @Override
    public V next() {
      return values.next().datum();
    }

    @Override
    public void remove() {
      throw new UnsupportedOperationException();
    }

    @Override
    public Iterator<V> iterator() {
      return this;
    }
  }

  private ReduceIterable reduceIterable = new ReduceIterable();

  @Override
  public final void reduce(AIngleKey<K> key, Iterator<AIngleValue<V>> values, OutputCollector<KO, VO> out,
      Reporter reporter) throws IOException {
    if (this.collector == null)
      this.collector = getCollector(out);
    reduceIterable.values = values;
    reducer.reduce(key.datum(), reduceIterable, collector, reporter);
  }

  @Override
  public void close() throws IOException {
    this.reducer.close();
  }
}
