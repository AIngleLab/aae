/*

 */

package org.apache.aingle.perf.test.basic;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.perf.test.BasicState;
import org.apache.aingle.util.Utf8;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.OperationsPerInvocation;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;

public class MapTest {

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public void encode(final TestStateEncode state) throws Exception {
    final Encoder e = state.encoder;
    final int items = state.getBatchSize() / 4;
    e.writeMapStart();
    e.setItemCount(items);
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      e.startItem();
      e.writeString(state.utf);
      e.writeFloat(state.testData[i + 0]);
      e.writeFloat(state.testData[i + 1]);
      e.writeFloat(state.testData[i + 2]);
      e.writeFloat(state.testData[i + 3]);
    }
    e.writeMapEnd();
  }

  @Benchmark
  public float decode(final TestStateDecode state) throws Exception {
    final Decoder d = state.decoder;
    float result = 0.0f;
    for (long i = d.readMapStart(); i != 0; i = d.mapNext()) {
      for (long j = 0; j < i; j++) {
        state.utf = d.readString(state.utf);
        result += d.readFloat();
        result += d.readFloat();
        result += d.readFloat();
        result += d.readFloat();
      }
    }
    return result;
  }

  @State(Scope.Thread)
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public static class TestStateEncode extends BasicState {

    private float[] testData;
    private Encoder encoder;
    private Utf8 utf = new Utf8("This is a map key");

    public TestStateEncode() {
      super();
    }

    /**
     * Setup each trial
     *
     * @throws IOException Could not setup test data
     */
    @Setup(Level.Trial)
    public void doSetupTrial() throws Exception {
      this.encoder = super.newEncoder(false, getNullOutputStream());
      this.testData = new float[getBatchSize()];

      for (int i = 0; i < testData.length; i++) {
        testData[i] = super.getRandom().nextFloat();
      }
    }
  }

  @State(Scope.Thread)
  public static class TestStateDecode extends BasicState {

    private byte[] testData;
    private Decoder decoder;
    private Utf8 utf = new Utf8();

    public TestStateDecode() {
      super();
    }

    /**
     * Generate test data.
     *
     * @throws IOException Could not setup test data
     */
    @Setup(Level.Trial)
    public void doSetupTrial() throws IOException {
      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      Encoder encoder = super.newEncoder(true, baos);
      final int items = getBatchSize() / 4;

      encoder.writeMapStart();
      encoder.setItemCount(items);
      for (int i = 0; i < getBatchSize(); i += 4) {
        encoder.startItem();
        encoder.writeString("This is a map key");
        encoder.writeFloat(super.getRandom().nextFloat());
        encoder.writeFloat(super.getRandom().nextFloat());
        encoder.writeFloat(super.getRandom().nextFloat());
        encoder.writeFloat(super.getRandom().nextFloat());
      }
      encoder.writeMapEnd();

      this.testData = baos.toByteArray();
    }

    @Setup(Level.Invocation)
    public void doSetupInvocation() throws Exception {
      this.decoder = super.newDecoder(this.testData);
    }
  }
}
