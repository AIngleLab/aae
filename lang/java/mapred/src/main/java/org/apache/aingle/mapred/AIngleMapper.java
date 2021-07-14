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
 * A mapper for AIngle data.
 *
 * <p>
 * Applications subclass this class and pass their subclass to
 * {@link AIngleJob#setMapperClass(JobConf, Class)}, overriding
 * {@link #map(Object, AIngleCollector, Reporter)}.
 */
public class AIngleMapper<IN, OUT> extends Configured implements JobConfigurable, Closeable {

  /** Called with each map input datum. By default, collects inputs. */
  @SuppressWarnings("unchecked")
  public void map(IN datum, AIngleCollector<OUT> collector, Reporter reporter) throws IOException {
    collector.collect((OUT) datum);
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
