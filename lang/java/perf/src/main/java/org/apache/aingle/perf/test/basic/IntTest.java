/*

 */

package org.apache.aingle.perf.test.basic;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.perf.test.BasicState;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.OperationsPerInvocation;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;

public class IntTest {

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public void encode(final TestStateEncode state) throws Exception {
    final Encoder e = state.encoder;
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      e.writeInt(state.testData[i + 0]);
      e.writeInt(state.testData[i + 1]);
      e.writeInt(state.testData[i + 2]);
      e.writeInt(state.testData[i + 3]);
    }
  }

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public int decode(final TestStateDecode state) throws Exception {
    final Decoder d = state.decoder;
    int total = 0;
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      total += d.readInt();
      total += d.readInt();
      total += d.readInt();
      total += d.readInt();
    }
    return total;
  }

  @State(Scope.Thread)
  public static class TestStateEncode extends BasicState {

    private int[] testData;
    private Encoder encoder;

    public TestStateEncode() {
      super();
    }

    /**
     * AIngle uses Zig-Zag variable length encoding for numeric values. Ensure there
     * are some numeric of each possible size.
     *
     * @throws IOException Could not setup test data
     */
    @Setup(Level.Trial)
    public void doSetupTrial() throws Exception {
      this.encoder = super.newEncoder(false, getNullOutputStream());
      this.testData = new int[getBatchSize()];

      for (int i = 0; i < testData.length; i += 4) {
        // fits in 1 byte
        testData[i + 0] = super.getRandom().nextInt(50);

        // fits in 2 bytes
        testData[i + 1] = super.getRandom().nextInt(5000);

        // fits in 3 bytes
        testData[i + 2] = super.getRandom().nextInt(500000);

        // most in 4 bytes, some in 5 bytes
        testData[i + 3] = super.getRandom().nextInt(150000000);
      }
    }
  }

  @State(Scope.Thread)
  public static class TestStateDecode extends BasicState {

    private byte[] testData;
    private Decoder decoder;

    public TestStateDecode() {
      super();
    }

    /**
     * AIngle uses Zig-Zag variable length encoding for numeric values. Ensure there
     * are some numeric values of each possible size.
     *
     * @throws IOException Could not setup test data
     */
    @Setup(Level.Trial)
    public void doSetupTrial() throws IOException {
      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      Encoder encoder = super.newEncoder(true, baos);

      for (int i = 0; i < getBatchSize(); i += 4) {
        // fits in 1 byte
        encoder.writeInt(super.getRandom().nextInt(50));

        // fits in 2 bytes
        encoder.writeInt(super.getRandom().nextInt(5000));

        // fits in 3 bytes
        encoder.writeInt(super.getRandom().nextInt(500000));

        // most in 4 bytes, some in 5 bytes
        encoder.writeInt(super.getRandom().nextInt(150000000));
      }

      this.testData = baos.toByteArray();
    }

    @Setup(Level.Invocation)
    public void doSetupInvocation() throws Exception {
      this.decoder = super.newDecoder(this.testData);
    }
  }
}
