/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.aingle.Schema;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.InputFormat;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.RecordReader;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.lib.MultipleInputs;
import org.apache.hadoop.util.ReflectionUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * An {@link InputFormat} that delegates read behavior of paths based on their
 * associated aingle schema.
 *
 * @see MultipleInputs#addInputPath(JobConf, Path, Class, Class)
 */
class DelegatingInputFormat<K, V> implements InputFormat<K, V> {
  private static final Logger LOG = LoggerFactory.getLogger(DelegatingInputFormat.class);

  @Override
  public InputSplit[] getSplits(JobConf conf, int numSplits) throws IOException {

    JobConf confCopy = new JobConf(conf);
    List<InputSplit> splits = new ArrayList<>();

    Map<Path, Class<? extends AIngleMapper>> mapperMap = AIngleMultipleInputs.getMapperTypeMap(conf);
    Map<Path, Schema> schemaMap = AIngleMultipleInputs.getInputSchemaMap(conf);
    Map<Schema, List<Path>> schemaPaths = new HashMap<>();

    // First, build a map of Schemas to Paths
    for (Entry<Path, Schema> entry : schemaMap.entrySet()) {
      if (!schemaPaths.containsKey(entry.getValue())) {
        schemaPaths.put(entry.getValue(), new ArrayList<>());
        LOG.info(entry.getValue().toString());
        LOG.info(String.valueOf(entry.getKey()));
      }

      schemaPaths.get(entry.getValue()).add(entry.getKey());
    }

    for (Entry<Schema, List<Path>> schemaEntry : schemaPaths.entrySet()) {
      Schema schema = schemaEntry.getKey();
      LOG.info(schema.toString());
      InputFormat format = ReflectionUtils.newInstance(AIngleInputFormat.class, conf);
      List<Path> paths = schemaEntry.getValue();

      Map<Class<? extends AIngleMapper>, List<Path>> mapperPaths = new HashMap<>();

      // Now, for each set of paths that have a common Schema, build
      // a map of Mappers to the paths they're used for
      for (Path path : paths) {
        Class<? extends AIngleMapper> mapperClass = mapperMap.get(path);
        if (!mapperPaths.containsKey(mapperClass)) {
          mapperPaths.put(mapperClass, new ArrayList<>());
        }

        mapperPaths.get(mapperClass).add(path);
      }

      // Now each set of paths that has a common InputFormat and Mapper can
      // be added to the same job, and split together.
      for (Entry<Class<? extends AIngleMapper>, List<Path>> mapEntry : mapperPaths.entrySet()) {
        paths = mapEntry.getValue();
        Class<? extends AIngleMapper> mapperClass = mapEntry.getKey();

        if (mapperClass == null) {
          mapperClass = (Class<? extends AIngleMapper>) conf.getMapperClass();
        }

        FileInputFormat.setInputPaths(confCopy, paths.toArray(new Path[0]));

        // Get splits for each input path and tag with InputFormat
        // and Mapper types by wrapping in a TaggedInputSplit.
        InputSplit[] pathSplits = format.getSplits(confCopy, numSplits);
        for (InputSplit pathSplit : pathSplits) {
          splits.add(new TaggedInputSplit(pathSplit, conf, format.getClass(), mapperClass, schema));
        }
      }
    }

    return splits.toArray(new InputSplit[0]);
  }

  @SuppressWarnings("unchecked")
  @Override
  public RecordReader<K, V> getRecordReader(InputSplit split, JobConf conf, Reporter reporter) throws IOException {

    // Find the Schema and then build the RecordReader from the
    // TaggedInputSplit.

    TaggedInputSplit taggedInputSplit = (TaggedInputSplit) split;
    Schema schema = taggedInputSplit.getSchema();
    AIngleJob.setInputSchema(conf, schema);
    InputFormat<K, V> inputFormat = (InputFormat<K, V>) ReflectionUtils
        .newInstance(taggedInputSplit.getInputFormatClass(), conf);
    return inputFormat.getRecordReader(taggedInputSplit.getInputSplit(), conf, reporter);
  }
}
