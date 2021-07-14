/*

 */

package org.apache.aingle.mapred;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.hadoop.io.serializer.Serialization;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.EncoderFactory;

/** The {@link Serialization} used by jobs configured with {@link AIngleJob}. */
public class AIngleSerialization<T> extends Configured implements Serialization<AIngleWrapper<T>> {

  @Override
  public boolean accept(Class<?> c) {
    return AIngleWrapper.class.isAssignableFrom(c);
  }

  /**
   * Returns the specified map output deserializer. Defaults to the final output
   * deserializer if no map output schema was specified.
   */
  @Override
  public Deserializer<AIngleWrapper<T>> getDeserializer(Class<AIngleWrapper<T>> c) {
    Configuration conf = getConf();
    boolean isKey = AIngleKey.class.isAssignableFrom(c);
    Schema schema = isKey ? Pair.getKeySchema(AIngleJob.getMapOutputSchema(conf))
        : Pair.getValueSchema(AIngleJob.getMapOutputSchema(conf));
    GenericData dataModel = AIngleJob.createMapOutputDataModel(conf);
    DatumReader<T> datumReader = dataModel.createDatumReader(schema);
    return new AIngleWrapperDeserializer(datumReader, isKey);
  }

  private static final DecoderFactory FACTORY = DecoderFactory.get();

  private class AIngleWrapperDeserializer implements Deserializer<AIngleWrapper<T>> {

    private DatumReader<T> reader;
    private BinaryDecoder decoder;
    private boolean isKey;

    public AIngleWrapperDeserializer(DatumReader<T> reader, boolean isKey) {
      this.reader = reader;
      this.isKey = isKey;
    }

    @Override
    public void open(InputStream in) {
      this.decoder = FACTORY.directBinaryDecoder(in, decoder);
    }

    @Override
    public AIngleWrapper<T> deserialize(AIngleWrapper<T> wrapper) throws IOException {
      T datum = reader.read(wrapper == null ? null : wrapper.datum(), decoder);
      if (wrapper == null) {
        wrapper = isKey ? new AIngleKey<>(datum) : new AIngleValue<>(datum);
      } else {
        wrapper.datum(datum);
      }
      return wrapper;
    }

    @Override
    public void close() throws IOException {
      decoder.inputStream().close();
    }

  }

  /** Returns the specified output serializer. */
  @Override
  public Serializer<AIngleWrapper<T>> getSerializer(Class<AIngleWrapper<T>> c) {
    // AIngleWrapper used for final output, AIngleKey or AIngleValue for map output
    boolean isFinalOutput = c.equals(AIngleWrapper.class);
    Configuration conf = getConf();
    Schema schema = isFinalOutput ? AIngleJob.getOutputSchema(conf)
        : (AIngleKey.class.isAssignableFrom(c) ? Pair.getKeySchema(AIngleJob.getMapOutputSchema(conf))
            : Pair.getValueSchema(AIngleJob.getMapOutputSchema(conf)));
    GenericData dataModel = AIngleJob.createDataModel(conf);
    return new AIngleWrapperSerializer(dataModel.createDatumWriter(schema));
  }

  private class AIngleWrapperSerializer implements Serializer<AIngleWrapper<T>> {

    private DatumWriter<T> writer;
    private OutputStream out;
    private BinaryEncoder encoder;

    public AIngleWrapperSerializer(DatumWriter<T> writer) {
      this.writer = writer;
    }

    @Override
    public void open(OutputStream out) {
      this.out = out;
      this.encoder = new EncoderFactory().binaryEncoder(out, null);
    }

    @Override
    public void serialize(AIngleWrapper<T> wrapper) throws IOException {
      writer.write(wrapper.datum(), encoder);
      // would be a lot faster if the Serializer interface had a flush()
      // method and the Hadoop framework called it when needed rather
      // than for every record.
      encoder.flush();
    }

    @Override
    public void close() throws IOException {
      out.close();
    }

  }

}
