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

package org.apache.aingle.hadoop.io;

import java.nio.ByteBuffer;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericFixed;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.aingle.mapred.AIngleWrapper;
import org.apache.aingle.mapreduce.AIngleJob;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.io.BooleanWritable;
import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.FloatWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.JobConf;

/**
 * Constructs converters that turn objects (usually from the output of a MR job)
 * into AIngle data that can be serialized.
 *
 * <p>
 * Currently, only the following types have implemented converters:
 * <ul>
 * <li>AIngleKey</li>
 * <li>AIngleValue</li>
 * <li>BooleanWritable</li>
 * <li>BytesWritable</li>
 * <li>ByteWritable</li>
 * <li>DoubleWritable</li>
 * <li>FloatWritable</li>
 * <li>IntWritable</li>
 * <li>LongWritable</li>
 * <li>NullWritable</li>
 * <li>Text</li>
 * </ul>
 * </p>
 */
public class AIngleDatumConverterFactory extends Configured {
  /**
   * Creates a new <code>AIngleDatumConverterFactory</code> instance.
   *
   * @param conf The job configuration.
   */
  public AIngleDatumConverterFactory(Configuration conf) {
    super(conf);
  }

  /**
   * Creates a converter that turns objects of type <code>inputClass</code> into
   * AIngle data.
   *
   * @param inputClass The type of input data to convert.
   * @return A converter that turns objects of type <code>inputClass</code> into
   *         AIngle data.
   */
  @SuppressWarnings("unchecked")
  public <IN, OUT> AIngleDatumConverter<IN, OUT> create(Class<IN> inputClass) {
    boolean isMapOnly = ((JobConf) getConf()).getNumReduceTasks() == 0;
    if (AIngleKey.class.isAssignableFrom(inputClass)) {
      Schema schema;
      if (isMapOnly) {
        schema = AIngleJob.getMapOutputKeySchema(getConf());
        if (null == schema) {
          schema = AIngleJob.getOutputKeySchema(getConf());
        }
      } else {
        schema = AIngleJob.getOutputKeySchema(getConf());
      }
      if (null == schema) {
        throw new IllegalStateException("Writer schema for output key was not set. Use AIngleJob.setOutputKeySchema().");
      }
      return (AIngleDatumConverter<IN, OUT>) new AIngleWrapperConverter(schema);
    }
    if (AIngleValue.class.isAssignableFrom(inputClass)) {
      Schema schema;
      if (isMapOnly) {
        schema = AIngleJob.getMapOutputValueSchema(getConf());
        if (null == schema) {
          schema = AIngleJob.getOutputValueSchema(getConf());
        }
      } else {
        schema = AIngleJob.getOutputValueSchema(getConf());
      }
      if (null == schema) {
        throw new IllegalStateException(
            "Writer schema for output value was not set. Use AIngleJob.setOutputValueSchema().");
      }
      return (AIngleDatumConverter<IN, OUT>) new AIngleWrapperConverter(schema);
    }
    if (BooleanWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new BooleanWritableConverter();
    }
    if (BytesWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new BytesWritableConverter();
    }
    if (ByteWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new ByteWritableConverter();
    }
    if (DoubleWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new DoubleWritableConverter();
    }
    if (FloatWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new FloatWritableConverter();
    }
    if (IntWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new IntWritableConverter();
    }
    if (LongWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new LongWritableConverter();
    }
    if (NullWritable.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new NullWritableConverter();
    }
    if (Text.class.isAssignableFrom(inputClass)) {
      return (AIngleDatumConverter<IN, OUT>) new TextConverter();
    }

    throw new UnsupportedOperationException("Unsupported input type: " + inputClass.getName());
  }

  /** Converts AIngleWrappers into their wrapped AIngle data. */
  public static class AIngleWrapperConverter extends AIngleDatumConverter<AIngleWrapper<?>, Object> {
    private final Schema mSchema;

    public AIngleWrapperConverter(Schema schema) {
      mSchema = schema;
    }

    /** {@inheritDoc} */
    @Override
    public Object convert(AIngleWrapper<?> input) {
      return input.datum();
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts BooleanWritables into Booleans. */
  public static class BooleanWritableConverter extends AIngleDatumConverter<BooleanWritable, Boolean> {
    private final Schema mSchema;

    /** Constructor. */
    public BooleanWritableConverter() {
      mSchema = Schema.create(Schema.Type.BOOLEAN);
    }

    /** {@inheritDoc} */
    @Override
    public Boolean convert(BooleanWritable input) {
      return input.get();
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts BytesWritables into ByteBuffers. */
  public static class BytesWritableConverter extends AIngleDatumConverter<BytesWritable, ByteBuffer> {
    private final Schema mSchema;

    /** Constructor. */
    public BytesWritableConverter() {
      mSchema = Schema.create(Schema.Type.BYTES);
    }

    /** {@inheritDoc} */
    @Override
    public ByteBuffer convert(BytesWritable input) {
      return ByteBuffer.wrap(input.getBytes());
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts ByteWritables into GenericFixed of size 1. */
  public static class ByteWritableConverter extends AIngleDatumConverter<ByteWritable, GenericFixed> {
    private final Schema mSchema;

    /** Constructor. */
    public ByteWritableConverter() {
      mSchema = Schema.createFixed("Byte", "A single byte", "org.apache.aingle.mapreduce", 1);
    }

    /** {@inheritDoc} */
    @Override
    public GenericFixed convert(ByteWritable input) {
      return new GenericData.Fixed(mSchema, new byte[] { input.get() });
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts DoubleWritables into Doubles. */
  public static class DoubleWritableConverter extends AIngleDatumConverter<DoubleWritable, Double> {
    private final Schema mSchema;

    /** Constructor. */
    public DoubleWritableConverter() {
      mSchema = Schema.create(Schema.Type.DOUBLE);
    }

    /** {@inheritDoc} */
    @Override
    public Double convert(DoubleWritable input) {
      return input.get();
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts FloatWritables into Floats. */
  public static class FloatWritableConverter extends AIngleDatumConverter<FloatWritable, Float> {
    private final Schema mSchema;

    /** Constructor. */
    public FloatWritableConverter() {
      mSchema = Schema.create(Schema.Type.FLOAT);
    }

    /** {@inheritDoc} */
    @Override
    public Float convert(FloatWritable input) {
      return input.get();
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts IntWritables into Ints. */
  public static class IntWritableConverter extends AIngleDatumConverter<IntWritable, Integer> {
    private final Schema mSchema;

    /** Constructor. */
    public IntWritableConverter() {
      mSchema = Schema.create(Schema.Type.INT);
    }

    /** {@inheritDoc} */
    @Override
    public Integer convert(IntWritable input) {
      return input.get();
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts LongWritables into Longs. */
  public static class LongWritableConverter extends AIngleDatumConverter<LongWritable, Long> {
    private final Schema mSchema;

    /** Constructor. */
    public LongWritableConverter() {
      mSchema = Schema.create(Schema.Type.LONG);
    }

    /** {@inheritDoc} */
    @Override
    public Long convert(LongWritable input) {
      return input.get();
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts NullWritables into Nulls. */
  public static class NullWritableConverter extends AIngleDatumConverter<NullWritable, Object> {
    private final Schema mSchema;

    /** Constructor. */
    public NullWritableConverter() {
      mSchema = Schema.create(Schema.Type.NULL);
    }

    /** {@inheritDoc} */
    @Override
    public Object convert(NullWritable input) {
      return null;
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }

  /** Converts Text into CharSequences. */
  public static class TextConverter extends AIngleDatumConverter<Text, CharSequence> {
    private final Schema mSchema;

    /** Constructor. */
    public TextConverter() {
      mSchema = Schema.create(Schema.Type.STRING);
    }

    /** {@inheritDoc} */
    @Override
    public CharSequence convert(Text input) {
      return input.toString();
    }

    /** {@inheritDoc} */
    @Override
    public Schema getWriterSchema() {
      return mSchema;
    }
  }
}
