/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

package org.apache.aingle.mapreduce;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.hadoop.io.AIngleKeyComparator;
import org.apache.aingle.hadoop.io.AIngleSerialization;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.Job;

/**
 * Utility methods for configuring jobs that work with AIngle.
 *
 * <p>
 * When using AIngle data as MapReduce keys and values, data must be wrapped in a
 * suitable AIngleWrapper implementation. MapReduce keys must be wrapped in an
 * AIngleKey object, and MapReduce values must be wrapped in an AIngleValue object.
 * </p>
 *
 * <p>
 * Suppose you would like to write a line count mapper that reads from a text
 * file. If instead of using a Text and IntWritable output value, you would like
 * to use AIngle data with a schema of <i>"string"</i> and <i>"int"</i>,
 * respectively, you may parametrize your mapper with
 * {@code AIngleKey<CharSequence>} and {@code AIngleValue<Integer>} types. Then, use
 * the <code>setMapOutputKeySchema()</code> and
 * <code>setMapOutputValueSchema()</code> methods to set writer schemas for the
 * records you will generate.
 * </p>
 */
public final class AIngleJob {
  /** Disable the constructor for this utility class. */
  private AIngleJob() {
  }

  /** Configuration key for the input key schema. */
  private static final String CONF_INPUT_KEY_SCHEMA = "aingle.schema.input.key";

  /** Configuration key for the input value schema. */
  private static final String CONF_INPUT_VALUE_SCHEMA = "aingle.schema.input.value";

  /** Configuration key for the output key schema. */
  private static final String CONF_OUTPUT_KEY_SCHEMA = "aingle.schema.output.key";

  /** Configuration key for the output value schema. */
  private static final String CONF_OUTPUT_VALUE_SCHEMA = "aingle.schema.output.value";

  /**
   * The configuration key for a job's output compression codec. This takes one of
   * the strings registered in {@link org.apache.aingle.file.CodecFactory}
   */
  public static final String CONF_OUTPUT_CODEC = "aingle.output.codec";

  /**
   * Sets the job input key schema.
   *
   * @param job    The job to configure.
   * @param schema The input key schema.
   */
  public static void setInputKeySchema(Job job, Schema schema) {
    job.getConfiguration().set(CONF_INPUT_KEY_SCHEMA, schema.toString());
  }

  /**
   * Sets the job input value schema.
   *
   * @param job    The job to configure.
   * @param schema The input value schema.
   */
  public static void setInputValueSchema(Job job, Schema schema) {
    job.getConfiguration().set(CONF_INPUT_VALUE_SCHEMA, schema.toString());
  }

  /**
   * Sets the map output key schema.
   *
   * @param job    The job to configure.
   * @param schema The map output key schema.
   */
  public static void setMapOutputKeySchema(Job job, Schema schema) {
    job.setMapOutputKeyClass(AIngleKey.class);
    job.setGroupingComparatorClass(AIngleKeyComparator.class);
    job.setSortComparatorClass(AIngleKeyComparator.class);
    AIngleSerialization.setKeyWriterSchema(job.getConfiguration(), schema);
    AIngleSerialization.setKeyReaderSchema(job.getConfiguration(), schema);
    AIngleSerialization.addToConfiguration(job.getConfiguration());
  }

  /**
   * Sets the map output value schema.
   *
   * @param job    The job to configure.
   * @param schema The map output value schema.
   */
  public static void setMapOutputValueSchema(Job job, Schema schema) {
    job.setMapOutputValueClass(AIngleValue.class);
    AIngleSerialization.setValueWriterSchema(job.getConfiguration(), schema);
    AIngleSerialization.setValueReaderSchema(job.getConfiguration(), schema);
    AIngleSerialization.addToConfiguration(job.getConfiguration());
  }

  /**
   * Sets the job output key schema.
   *
   * @param job    The job to configure.
   * @param schema The job output key schema.
   */
  public static void setOutputKeySchema(Job job, Schema schema) {
    job.setOutputKeyClass(AIngleKey.class);
    job.getConfiguration().set(CONF_OUTPUT_KEY_SCHEMA, schema.toString());
  }

  /**
   * Sets the job output value schema.
   *
   * @param job    The job to configure.
   * @param schema The job output value schema.
   */
  public static void setOutputValueSchema(Job job, Schema schema) {
    job.setOutputValueClass(AIngleValue.class);
    job.getConfiguration().set(CONF_OUTPUT_VALUE_SCHEMA, schema.toString());
  }

  /**
   * Sets the job data model class.
   *
   * @param job        The job to configure.
   * @param modelClass The job data model class.
   */
  public static void setDataModelClass(Job job, Class<? extends GenericData> modelClass) {
    AIngleSerialization.setDataModelClass(job.getConfiguration(), modelClass);
  }

  /**
   * Gets the job input key schema.
   *
   * @param conf The job configuration.
   * @return The job input key schema, or null if not set.
   */
  public static Schema getInputKeySchema(Configuration conf) {
    String schemaString = conf.get(CONF_INPUT_KEY_SCHEMA);
    return schemaString != null ? new Schema.Parser().parse(schemaString) : null;
  }

  /**
   * Gets the job input value schema.
   *
   * @param conf The job configuration.
   * @return The job input value schema, or null if not set.
   */
  public static Schema getInputValueSchema(Configuration conf) {
    String schemaString = conf.get(CONF_INPUT_VALUE_SCHEMA);
    return schemaString != null ? new Schema.Parser().parse(schemaString) : null;
  }

  /**
   * Gets the map output key schema.
   *
   * @param conf The job configuration.
   * @return The map output key schema, or null if not set.
   */
  public static Schema getMapOutputKeySchema(Configuration conf) {
    return AIngleSerialization.getKeyWriterSchema(conf);
  }

  /**
   * Gets the map output value schema.
   *
   * @param conf The job configuration.
   * @return The map output value schema, or null if not set.
   */
  public static Schema getMapOutputValueSchema(Configuration conf) {
    return AIngleSerialization.getValueWriterSchema(conf);
  }

  /**
   * Gets the job output key schema.
   *
   * @param conf The job configuration.
   * @return The job output key schema, or null if not set.
   */
  public static Schema getOutputKeySchema(Configuration conf) {
    String schemaString = conf.get(CONF_OUTPUT_KEY_SCHEMA);
    return schemaString != null ? new Schema.Parser().parse(schemaString) : null;
  }

  /**
   * Gets the job output value schema.
   *
   * @param conf The job configuration.
   * @return The job output value schema, or null if not set.
   */
  public static Schema getOutputValueSchema(Configuration conf) {
    String schemaString = conf.get(CONF_OUTPUT_VALUE_SCHEMA);
    return schemaString != null ? new Schema.Parser().parse(schemaString) : null;
  }
}
