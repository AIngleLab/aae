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

public class StringTest {

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public void encode(final TestStateEncode state) throws Exception {
    final Encoder e = state.encoder;
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      e.writeString(state.testData[i + 0]);
      e.writeString(state.testData[i + 1]);
      e.writeString(state.testData[i + 2]);
      e.writeString(state.testData[i + 3]);
    }
  }

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public int decode(final TestStateDecode state) throws Exception {
    final Decoder d = state.decoder;
    int result = 0;
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      result += d.readString(state.utf).toString().length();
      result += d.readString(state.utf).toString().length();
      result += d.readString(state.utf).toString().length();
      result += d.readString(state.utf).toString().length();
    }
    return result;
  }

  @State(Scope.Thread)
  public static class TestStateEncode extends BasicState {

    private String[] testData;
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
      this.testData = new String[getBatchSize()];

      for (int i = 0; i < testData.length; i++) {
        testData[i] = randomString();
      }
    }

    private String randomString() {
      final char[] data = new char[super.getRandom().nextInt(70)];
      for (int j = 0; j < data.length; j++) {
        data[j] = (char) ('a' + super.getRandom().nextInt('z' - 'a'));
      }
      return new String(data);
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

      for (int i = 0; i < getBatchSize(); i++) {
        encoder.writeString(randomString());
      }

      this.testData = baos.toByteArray();
    }

    private String randomString() {
      final char[] data = new char[super.getRandom().nextInt(70)];
      for (int j = 0; j < data.length; j++) {
        data[j] = (char) ('a' + super.getRandom().nextInt('z' - 'a'));
      }
      return new String(data);
    }

    @Setup(Level.Invocation)
    public void doSetupInvocation() throws Exception {
      this.decoder = super.newDecoder(this.testData);
    }
  }
}
