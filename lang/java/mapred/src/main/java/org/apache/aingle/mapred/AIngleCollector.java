/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.conf.Configured;

/** A collector for map and reduce output. */
public abstract class AIngleCollector<T> extends Configured {
  public abstract void collect(T datum) throws IOException;
}
