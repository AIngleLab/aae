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
 * An {@link Mapper} that delegates behaviour of paths to multiple other
 * mappers. Similar to {@link HadoopMapper}, but instantiates map classes in the
 * map() call instead of during configure(), as we rely on the split object to
 * provide us that information.
 *
 * @see {@link AIngleMultipleInputs#addInputPath(JobConf, Path, Class, Schema)}
 */
class DelegatingMapper<IN, OUT, K, V, KO, VO> extends MapReduceBase
    implements Mapper<AIngleWrapper<IN>, NullWritable, KO, VO> {
  AIngleMapper<IN, OUT> mapper;
  JobConf conf;
  boolean isMapOnly;
  AIngleCollector<OUT> out;

  @Override
  public void configure(JobConf conf) {
    this.conf = conf;
    this.isMapOnly = conf.getNumReduceTasks() == 0;
  }

  @Override
  public void map(AIngleWrapper<IN> wrapper, NullWritable value, OutputCollector<KO, VO> collector, Reporter reporter)
      throws IOException {
    if (mapper == null) {
      TaggedInputSplit is = (TaggedInputSplit) reporter.getInputSplit();
      Class<? extends AIngleMapper> mapperClass = is.getMapperClass();
      mapper = (AIngleMapper<IN, OUT>) ReflectionUtils.newInstance(mapperClass, conf);
    }
    if (out == null)
      out = new MapCollector<OUT, K, V, KO, VO>(collector, isMapOnly);
    mapper.map(wrapper.datum(), out, reporter);
  }
}
