/*

 */

package org.apache.aingle.perf.test.basic;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.perf.test.BasicState;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.OperationsPerInvocation;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;

public class BytesTest {

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public void encode(final TestStateEncode state) throws Exception {
    final Encoder e = state.encoder;
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      e.writeBytes(state.testData[i + 0]);
      e.writeBytes(state.testData[i + 1]);
      e.writeBytes(state.testData[i + 2]);
      e.writeBytes(state.testData[i + 3]);
    }
  }

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public ByteBuffer decode(final TestStateDecode state) throws Exception {
    final Decoder d = state.decoder;
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      d.readBytes(state.bb);
      d.readBytes(state.bb);
      d.readBytes(state.bb);
      d.readBytes(state.bb);
    }
    return state.bb;
  }

  @State(Scope.Thread)
  public static class TestStateEncode extends BasicState {

    private byte[][] testData;
    private Encoder encoder;

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
      this.testData = new byte[getBatchSize()][];

      for (int i = 0; i < testData.length; i++) {
        final byte[] data = new byte[super.getRandom().nextInt(70)];
        super.getRandom().nextBytes(data);
        testData[i] = data;
      }
    }
  }

  @State(Scope.Thread)
  public static class TestStateDecode extends BasicState {

    private byte[] testData;
    private Decoder decoder;
    private ByteBuffer bb = ByteBuffer.allocate(70);

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

      for (int i = 0; i < getBatchSize(); i++) {
        final byte[] data = new byte[super.getRandom().nextInt(70)];
        super.getRandom().nextBytes(data);
        encoder.writeBytes(data);
      }

      this.testData = baos.toByteArray();
    }

    @Setup(Level.Invocation)
    public void doSetupInvocation() throws Exception {
      this.decoder = super.newDecoder(this.testData);
    }
  }
}
