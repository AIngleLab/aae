/*

 */

package org.apache.aingle.mapred;

import java.io.Closeable;
import java.io.IOException;

import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobConfigurable;
import org.apache.hadoop.mapred.Reporter;

/**
 * A reducer for AIngle data.
 *
 * <p>
 * Applications should subclass this class and pass their subclass to
 * {@link AIngleJob#setReducerClass(JobConf, Class)} and perhaps
 * {@link AIngleJob#setCombinerClass(JobConf, Class)}. Subclasses override
 * {@link #reduce(Object, Iterable, AIngleCollector, Reporter)}.
 */

public class AIngleReducer<K, V, OUT> extends Configured implements JobConfigurable, Closeable {

  private Pair<K, V> outputPair;

  /**
   * Called with all map output values with a given key. By default, pairs key
   * with each value, collecting {@link Pair} instances.
   */
  @SuppressWarnings("unchecked")
  public void reduce(K key, Iterable<V> values, AIngleCollector<OUT> collector, Reporter reporter) throws IOException {
    if (outputPair == null)
      outputPair = new Pair<>(AIngleJob.getOutputSchema(getConf()));
    for (V value : values) {
      outputPair.set(key, value);
      collector.collect((OUT) outputPair);
    }
  }

  /** Subclasses can override this as desired. */
  @Override
  public void close() throws IOException {
    // no op
  }

  /** Subclasses can override this as desired. */
  @Override
  public void configure(JobConf jobConf) {
    // no op
  }
}
