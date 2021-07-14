/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.util.ReflectionUtils;

/**
 * Bridge between a {@link org.apache.hadoop.mapred.Mapper} and an
 * {@link AIngleMapper}. Outputs are written directly when a job is map-only, but
 * are otherwise assumed to be pairs that are split.
 */
class HadoopMapper<IN, OUT, K, V, KO, VO> extends MapReduceBase
    implements Mapper<AIngleWrapper<IN>, NullWritable, KO, VO> {

  private AIngleMapper<IN, OUT> mapper;
  private MapCollector<OUT, K, V, KO, VO> out;
  private boolean isMapOnly;

  @Override
  @SuppressWarnings("unchecked")
  public void configure(JobConf conf) {
    this.mapper = ReflectionUtils.newInstance(conf.getClass(AIngleJob.MAPPER, AIngleMapper.class, AIngleMapper.class), conf);
    this.isMapOnly = conf.getNumReduceTasks() == 0;
  }

  @Override
  public void map(AIngleWrapper<IN> wrapper, NullWritable value, OutputCollector<KO, VO> collector, Reporter reporter)
      throws IOException {
    if (this.out == null)
      this.out = new MapCollector<>(collector, isMapOnly);
    mapper.map(wrapper.datum(), out, reporter);
  }

  @Override
  public void close() throws IOException {
    this.mapper.close();
  }

}
