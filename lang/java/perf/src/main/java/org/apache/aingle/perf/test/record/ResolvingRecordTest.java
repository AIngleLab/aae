/*

 */

package org.apache.aingle.perf.test.record;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.perf.test.BasicRecord;
import org.apache.aingle.perf.test.BasicState;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.OperationsPerInvocation;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;

public class ResolvingRecordTest {

  private static final String RECORD_SCHEMA = "{ \"type\": \"record\", \"name\": \"R\", \"fields\": [\n"
      + "{ \"name\": \"f1\", \"type\": \"double\" },\n" + "{ \"name\": \"f2\", \"type\": \"double\" },\n"
      + "{ \"name\": \"f3\", \"type\": \"double\" },\n" + "{ \"name\": \"f4\", \"type\": \"int\" },\n"
      + "{ \"name\": \"f5\", \"type\": \"int\" },\n" + "{ \"name\": \"f6\", \"type\": \"int\" }\n" + "] }";

  private static final String RECORD_SCHEMA_WITH_OUT_OF_ORDER = "{ \"type\": \"record\", \"name\": \"R\", \"fields\": [\n"
      + "{ \"name\": \"f1\", \"type\": \"double\" },\n" + "{ \"name\": \"f3\", \"type\": \"double\" },\n"
      + "{ \"name\": \"f5\", \"type\": \"int\" },\n" + "{ \"name\": \"f2\", \"type\": \"double\" },\n"
      + "{ \"name\": \"f4\", \"type\": \"int\" },\n" + "{ \"name\": \"f6\", \"type\": \"int\" }\n" + "] }";

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public void decode(final TestStateDecode state) throws Exception {
    final Decoder d = state.decoder;
    for (int i = 0; i < state.getBatchSize(); i++) {
      // TODO: Would expect this to be D,D,I,D,I,I to match read schema
      d.readDouble();
      d.readDouble();
      d.readDouble();
      d.readInt();
      d.readInt();
      d.readInt();
    }
  }

  @State(Scope.Thread)
  public static class TestStateDecode extends BasicState {

    private final Schema writerSchema;
    private final Schema readerSchema;

    private byte[] testData;
    private Decoder decoder;

    public TestStateDecode() {
      super();
      this.writerSchema = new Schema.Parser().parse(RECORD_SCHEMA);
      this.readerSchema = new Schema.Parser().parse(RECORD_SCHEMA_WITH_OUT_OF_ORDER);
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
        final BasicRecord r = new BasicRecord(super.getRandom());
        encoder.writeDouble(r.f1);
        encoder.writeDouble(r.f2);
        encoder.writeDouble(r.f3);
        encoder.writeInt(r.f4);
        encoder.writeInt(r.f5);
        encoder.writeInt(r.f6);
      }

      this.testData = baos.toByteArray();
    }

    @Setup(Level.Invocation)
    public void doSetupInvocation() throws Exception {
      this.decoder = DecoderFactory.get().resolvingDecoder(writerSchema, readerSchema, super.newDecoder(this.testData));
    }
  }
}
