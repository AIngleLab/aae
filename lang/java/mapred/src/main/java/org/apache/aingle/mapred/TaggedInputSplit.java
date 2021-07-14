/*

 */

package org.apache.aingle.mapred;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.hadoop.conf.Configurable;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.InputFormat;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.util.ReflectionUtils;

/**
 * An {@link InputSplit} that tags another InputSplit with extra data for use by
 * {@link DelegatingInputFormat}s and {@link DelegatingMapper}s.
 */
class TaggedInputSplit implements Configurable, InputSplit {

  private Class<? extends InputSplit> inputSplitClass;

  private InputSplit inputSplit;

  private Class<? extends InputFormat> inputFormatClass;

  private Class<? extends AIngleMapper> mapperClass;

  private Schema schema;

  private Schema.Parser schemaParser = new Schema.Parser();

  private Configuration conf;

  public TaggedInputSplit() {
    // Default constructor.
  }

  /**
   * Creates a new TaggedInputSplit.
   *
   * @param inputSplit       The InputSplit to be tagged
   * @param conf             The configuration to use
   * @param inputFormatClass The InputFormat class to use for this job
   * @param mapperClass      The Mapper class to use for this job
   */
  public TaggedInputSplit(InputSplit inputSplit, Configuration conf, Class<? extends InputFormat> inputFormatClass,
      Class<? extends AIngleMapper> mapperClass, Schema inputSchema) {
    this.inputSplitClass = inputSplit.getClass();
    this.inputSplit = inputSplit;
    this.conf = conf;
    this.inputFormatClass = inputFormatClass;
    this.mapperClass = mapperClass;
    this.schema = inputSchema;
  }

  /**
   * Retrieves the original InputSplit.
   *
   * @return The InputSplit that was tagged
   */
  public InputSplit getInputSplit() {
    return inputSplit;
  }

  /**
   * Retrieves the InputFormat class to use for this split.
   *
   * @return The InputFormat class to use
   */
  public Class<? extends InputFormat> getInputFormatClass() {
    return inputFormatClass;
  }

  /**
   * Retrieves the Mapper class to use for this split.
   *
   * @return The Mapper class to use
   */
  public Class<? extends AIngleMapper> getMapperClass() {
    return mapperClass;
  }

  /**
   * Retrieves the Schema to use for this split.
   *
   * @return The schema for record readers to use
   */
  public Schema getSchema() {
    return schema;
  }

  @Override
  public long getLength() throws IOException {
    return inputSplit.getLength();
  }

  @Override
  public String[] getLocations() throws IOException {
    return inputSplit.getLocations();
  }

  @SuppressWarnings("unchecked")
  @Override
  public void readFields(DataInput in) throws IOException {
    inputSplitClass = (Class<? extends InputSplit>) readClass(in);
    inputSplit = ReflectionUtils.newInstance(inputSplitClass, conf);
    inputSplit.readFields(in);
    inputFormatClass = (Class<? extends InputFormat>) readClass(in);
    mapperClass = (Class<? extends AIngleMapper>) readClass(in);
    String schemaString = Text.readString(in);
    schema = schemaParser.parse(schemaString);
  }

  private Class<?> readClass(DataInput in) throws IOException {
    String className = Text.readString(in);
    try {
      return conf.getClassByName(className);
    } catch (ClassNotFoundException e) {
      throw new RuntimeException("readObject can't find class", e);
    }
  }

  @Override
  public void write(DataOutput out) throws IOException {
    Text.writeString(out, inputSplitClass.getName());
    inputSplit.write(out);
    Text.writeString(out, inputFormatClass.getName());
    Text.writeString(out, mapperClass.getName());
    Text.writeString(out, schema.toString());
  }

  @Override
  public Configuration getConf() {
    return conf;
  }

  @Override
  public void setConf(Configuration conf) {
    this.conf = conf;
  }

  @Override
  public String toString() {
    return inputSplit.toString();
  }
}
