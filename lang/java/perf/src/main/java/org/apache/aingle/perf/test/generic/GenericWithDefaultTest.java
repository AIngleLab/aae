/*

 */

package org.apache.aingle.perf.test.generic;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Random;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.perf.test.BasicState;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.infra.Blackhole;

public class GenericWithDefaultTest {

  private static final String RECORD_SCHEMA_WITH_DEFAULT = "{ \"type\": \"record\", \"name\": \"R\", \"fields\": [\n"
      + "{ \"name\": \"f1\", \"type\": \"double\" },\n" + "{ \"name\": \"f2\", \"type\": \"double\" },\n"
      + "{ \"name\": \"f3\", \"type\": \"double\" },\n" + "{ \"name\": \"f4\", \"type\": \"int\" },\n"
      + "{ \"name\": \"f5\", \"type\": \"int\" },\n" + "{ \"name\": \"f6\", \"type\": \"int\" },\n"
      + "{ \"name\": \"f7\", \"type\": \"string\", " + "\"default\": \"undefined\" },\n"
      + "{ \"name\": \"f8\", \"type\": \"string\"," + "\"default\": \"undefined\" }\n" + "] }";

  @Benchmark
  public void encode(final TestStateEncode state) throws Exception {
    final Encoder e = state.encoder;
    final GenericDatumWriter<Object> writer = new GenericDatumWriter<>(state.schema);
    for (final GenericRecord rec : state.testData) {
      writer.write(rec, e);
    }
  }

  @Benchmark
  public void decode(final Blackhole blackhole, final TestStateDecode state) throws Exception {
    final Decoder d = state.decoder;
    final GenericDatumReader<Object> reader = new GenericDatumReader<>(state.schema);
    for (int i = 0; i < state.getBatchSize(); i++) {
      blackhole.consume(reader.read(null, d));
    }
  }

  @State(Scope.Thread)
  public static class TestStateEncode extends BasicState {

    private final Schema schema;

    private GenericRecord[] testData;
    private Encoder encoder;

    public TestStateEncode() {
      super();
      this.schema = new Schema.Parser().parse(RECORD_SCHEMA_WITH_DEFAULT);
    }

    /**
     * Setup the trial data.
     *
     * @throws IOException Could not setup test data
     */
    @Setup(Level.Trial)
    public void doSetupTrial() throws Exception {
      this.encoder = super.newEncoder(false, getNullOutputStream());
      this.testData = new GenericRecord[getBatchSize()];

      final Random r = super.getRandom();
      for (int i = 0; i < testData.length; i++) {
        final GenericRecord rec = new GenericData.Record(schema);
        rec.put(0, r.nextDouble());
        rec.put(1, r.nextDouble());
        rec.put(2, r.nextDouble());
        rec.put(3, r.nextInt());
        rec.put(4, r.nextInt());
        rec.put(5, r.nextInt());
        rec.put(6, randomString(r));
        rec.put(7, randomString(r));
        testData[i] = rec;
      }
    }
  }

  @State(Scope.Thread)
  public static class TestStateDecode extends BasicState {

    private final Schema schema;

    private byte[] testData;
    private Decoder decoder;

    public TestStateDecode() {
      super();
      this.schema = new Schema.Parser().parse(RECORD_SCHEMA_WITH_DEFAULT);
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

      final GenericDatumWriter<Object> writer = new GenericDatumWriter<>(this.schema);

      final Random r = super.getRandom();
      for (int i = 0; i < getBatchSize(); i++) {
        final GenericRecord rec = new GenericData.Record(schema);
        rec.put(0, r.nextDouble());
        rec.put(1, r.nextDouble());
        rec.put(2, r.nextDouble());
        rec.put(3, r.nextInt());
        rec.put(4, r.nextInt());
        rec.put(5, r.nextInt());
        rec.put(6, randomString(r));
        rec.put(7, randomString(r));
        writer.write(rec, encoder);
      }

      encoder.flush();

      this.testData = baos.toByteArray();
    }

    @Setup(Level.Invocation)
    public void doSetupInvocation() throws Exception {
      this.decoder = DecoderFactory.get().validatingDecoder(schema, super.newDecoder(this.testData));
    }
  }

  private static String randomString(Random r) {
    char[] data = new char[r.nextInt(70)];
    for (int j = 0; j < data.length; j++) {
      data[j] = (char) ('a' + r.nextInt('z' - 'a'));
    }
    return new String(data);
  }
}
