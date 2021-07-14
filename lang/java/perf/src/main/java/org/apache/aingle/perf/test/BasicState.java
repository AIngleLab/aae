/*

 */

package org.apache.aingle.perf.test;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Random;

import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;

public abstract class BasicState {

  public static final int BATCH_SIZE = 10000;

  private static final DecoderFactory DECODER_FACTORY = new DecoderFactory();
  private static final EncoderFactory ENCODER_FACTORY = new EncoderFactory();

  private static final OutputStream NULL_OUTPUTSTREAM = new NullOutputStream();

  private final Random random = new Random(13L);
  private final int batchSize = BATCH_SIZE;

  private BinaryDecoder reuseDecoder;
  private BinaryEncoder reuseEncoder;
  private BinaryEncoder reuseBlockingEncoder;

  public BasicState() {
    this.reuseDecoder = null;
  }

  protected Random getRandom() {
    return this.random;
  }

  protected Decoder newDecoder(final byte[] buf) {
    this.reuseDecoder = DECODER_FACTORY.binaryDecoder(buf, this.reuseDecoder);
    return this.reuseDecoder;
  }

  protected Encoder newEncoder(boolean direct, OutputStream out) throws IOException {
    this.reuseEncoder = (direct ? ENCODER_FACTORY.directBinaryEncoder(out, this.reuseEncoder)
        : ENCODER_FACTORY.binaryEncoder(out, this.reuseEncoder));
    return this.reuseEncoder;
  }

  protected Encoder newEncoder(int blockSize, OutputStream out) throws IOException {
    this.reuseBlockingEncoder = ENCODER_FACTORY.configureBlockSize(blockSize).blockingBinaryEncoder(out,
        this.reuseBlockingEncoder);
    return this.reuseBlockingEncoder;
  }

  public int getBatchSize() {
    return this.batchSize;
  }

  protected OutputStream getNullOutputStream() {
    return NULL_OUTPUTSTREAM;
  }

  private static class NullOutputStream extends OutputStream {
    @Override
    public void write(int b) throws IOException {
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
    }

    @Override
    public void write(byte[] b) throws IOException {
    }
  }

}
