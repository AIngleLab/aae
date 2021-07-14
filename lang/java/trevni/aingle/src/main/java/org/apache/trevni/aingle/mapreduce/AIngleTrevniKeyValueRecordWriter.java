/*

 */

package org.apache.trevni.aingle.mapreduce;

import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.hadoop.io.AIngleDatumConverter;
import org.apache.aingle.hadoop.io.AIngleDatumConverterFactory;
import org.apache.aingle.hadoop.io.AIngleKeyValue;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapred.AIngleValue;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

/**
 * Writes key/value pairs to an Trevni container file.
 *
 * <p>
 * Each entry in the Trevni container file will be a generic record with two
 * fields, named 'key' and 'value'. The input types may be basic Writable
 * objects like Text or IntWritable, or they may be AIngleWrapper subclasses
 * (AIngleKey or AIngleValue). Writable objects will be converted to their
 * corresponding AIngle types when written to the generic record key/value pair.
 * </p>
 *
 * @param <K> The type of key to write.
 * @param <V> The type of value to write.
 */
public class AIngleTrevniKeyValueRecordWriter<K, V>
    extends AIngleTrevniRecordWriterBase<AIngleKey<K>, AIngleValue<V>, GenericRecord> {

  /**
   * The writer schema for the generic record entries of the Trevni container
   * file.
   */
  Schema mKeyValuePairSchema;

  /** A reusable AIngle generic record for writing key/value pairs to the file. */
  AIngleKeyValue<Object, Object> keyValueRecord;

  /** A helper object that converts the input key to an AIngle datum. */
  AIngleDatumConverter<K, ?> keyConverter;

  /** A helper object that converts the input value to an AIngle datum. */
  AIngleDatumConverter<V, ?> valueConverter;

  /**
   * Constructor.
   * 
   * @param context The TaskAttempContext to supply the writer with information
   *                form the job configuration
   */
  public AIngleTrevniKeyValueRecordWriter(TaskAttemptContext context) throws IOException {
    super(context);

    mKeyValuePairSchema = initSchema(context);
    keyValueRecord = new AIngleKeyValue<>(new GenericData.Record(mKeyValuePairSchema));
  }

  /** {@inheritDoc} */
  @Override
  public void write(AIngleKey<K> key, AIngleValue<V> value) throws IOException, InterruptedException {

    keyValueRecord.setKey(key.datum());
    keyValueRecord.setValue(value.datum());
    writer.write(keyValueRecord.get());
    if (writer.sizeEstimate() >= blockSize) // block full
      flush();
  }

  /** {@inheritDoc} */
  @SuppressWarnings("unchecked")
  @Override
  protected Schema initSchema(TaskAttemptContext context) {
    AIngleDatumConverterFactory converterFactory = new AIngleDatumConverterFactory(context.getConfiguration());

    keyConverter = converterFactory.create((Class<K>) context.getOutputKeyClass());
    valueConverter = converterFactory.create((Class<V>) context.getOutputValueClass());

    // Create the generic record schema for the key/value pair.
    return AIngleKeyValue.getSchema(keyConverter.getWriterSchema(), valueConverter.getWriterSchema());

  }

}
