/*

 */

package org.apache.aingle.mapred.tether;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.aingle.mapred.Pair;
import org.apache.aingle.util.Utf8;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Example Java tethered mapreduce executable. Implements map and reduce
 * functions for word count.
 */
public class WordCountTask extends TetherTask<Utf8, Pair<Utf8, Long>, Pair<Utf8, Long>> {

  static final Logger LOG = LoggerFactory.getLogger(WordCountTask.class);

  @Override
  public void map(Utf8 text, Collector<Pair<Utf8, Long>> collector) throws IOException {
    StringTokenizer tokens = new StringTokenizer(text.toString());
    while (tokens.hasMoreTokens())
      collector.collect(new Pair<>(new Utf8(tokens.nextToken()), 1L));
  }

  private long sum;

  @Override
  public void reduce(Pair<Utf8, Long> wc, Collector<Pair<Utf8, Long>> c) {
    sum += wc.value();
  }

  @Override
  public void reduceFlush(Pair<Utf8, Long> wc, Collector<Pair<Utf8, Long>> c) throws IOException {
    wc.value(sum);
    c.collect(wc);
    sum = 0;
  }

  public static void main(String[] args) throws Exception {
    new TetherTaskRunner(new WordCountTask()).join();
    LOG.info("WordCountTask finished");
  }

}
