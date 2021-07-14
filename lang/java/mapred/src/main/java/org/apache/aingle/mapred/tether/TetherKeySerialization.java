/*

 */

package org.apache.aingle.mapred.tether;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.hadoop.io.serializer.Serialization;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.conf.Configured;

import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.EncoderFactory;

/** A {@link Serialization} for {@link TetherData}. */
class TetherKeySerialization extends Configured implements Serialization<TetherData> {

  public boolean accept(Class<?> c) {
    return TetherData.class.isAssignableFrom(c);
  }

  public Deserializer<TetherData> getDeserializer(Class<TetherData> c) {
    return new TetherDataDeserializer();
  }

  private static final DecoderFactory FACTORY = DecoderFactory.get();

  private class TetherDataDeserializer implements Deserializer<TetherData> {
    private BinaryDecoder decoder;

    public void open(InputStream in) {
      this.decoder = FACTORY.directBinaryDecoder(in, decoder);
    }

    public TetherData deserialize(TetherData datum) throws IOException {
      if (datum == null)
        datum = new TetherData();
      datum.buffer(decoder.readBytes(datum.buffer()));
      return datum;
    }

    public void close() throws IOException {
      decoder.inputStream().close();
    }
  }

  public Serializer<TetherData> getSerializer(Class<TetherData> c) {
    return new TetherDataSerializer();
  }

  private class TetherDataSerializer implements Serializer<TetherData> {

    private OutputStream out;
    private BinaryEncoder encoder;

    public void open(OutputStream out) {
      this.out = out;
      this.encoder = EncoderFactory.get().directBinaryEncoder(out, encoder);
    }

    public void serialize(TetherData datum) throws IOException {
      encoder.writeBytes(datum.buffer());
      encoder.flush(); // Flush shouldn't be required. Might be a bug in AINGLE.
    }

    public void close() throws IOException {
      encoder.flush();
      out.close();
    }

  }

}
