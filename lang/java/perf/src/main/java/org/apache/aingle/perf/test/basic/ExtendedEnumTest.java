/*

 */

package org.apache.aingle.perf.test.basic;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.perf.test.BasicState;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.OperationsPerInvocation;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.infra.Blackhole;

public class ExtendedEnumTest {

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public void encode(final TestStateEncode state) throws Exception {
    final Encoder e = state.encoder;
    GenericDatumWriter<Object> writer = new GenericDatumWriter<>(state.schema);
    for (int i = 0; i < state.getBatchSize(); i += 4) {
      writer.write(state.testData[i + 0], e);
      writer.write(state.testData[i + 1], e);
      writer.write(state.testData[i + 2], e);
      writer.write(state.testData[i + 3], e);
    }
  }

  @Benchmark
  @OperationsPerInvocation(BasicState.BATCH_SIZE)
  public void decode(final Blackhole blackHole, final TestStateDecode state) throws Exception {
    final Decoder d = state.decoder;
    final GenericDatumReader<Object> reader = new GenericDatumReader<>(state.schema);
    for (int i = 0; i < state.getBatchSize(); i++) {
      final Object o = reader.read(null, d);
      blackHole.consume(o);
    }
  }

  @State(Scope.Thread)
  public static class TestStateEncode extends BasicState {
    private static final String ENUM_SCHEMA = "{ \"type\": \"enum\", \"name\":\"E\", \"symbols\": [\"A\", \"B\"] }";

    private final Schema schema;
    private GenericRecord[] testData;
    private Encoder encoder;

    public TestStateEncode() {
      super();
      this.schema = new Schema.Parser().parse(mkSchema(ENUM_SCHEMA));
    }

    /**
     * Setup each trial
     *
     * @throws IOException Could not setup test data
     */
    @Setup(Level.Trial)
    public void doSetupTrial() throws Exception {
      this.encoder = super.newEncoder(false, getNullOutputStream());
      this.testData = new GenericRecord[getBatchSize()];

      final Schema enumSchema = this.schema.getField("f").schema();
      for (int i = 0; i < getBatchSize(); i++) {
        final GenericRecord rec = new GenericData.Record(this.schema);
        final int tag = super.getRandom().nextInt(2);

        rec.put("f", GenericData.get().createEnum(enumSchema.getEnumSymbols().get(tag), enumSchema));
        this.testData[i] = rec;
      }
    }

    private String mkSchema(String subschema) {
      return ("{ \"type\": \"record\", \"name\": \"R\", \"fields\": [\n" + "{ \"name\": \"f\", \"type\": " + subschema
          + "}\n" + "] }");
    }
  }

  @State(Scope.Thread)
  public static class TestStateDecode extends BasicState {
    private static final String ENUM_SCHEMA = "{ \"type\": \"enum\", \"name\":\"E\", \"symbols\": [\"A\",\"B\",\"C\",\"D\",\"E\"] }";

    private final Schema schema;

    private byte[] testData;
    private Decoder decoder;

    public TestStateDecode() {
      super();
      this.schema = new Schema.Parser().parse(mkSchema(ENUM_SCHEMA));
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
      final Schema enumSchema = this.schema.getField("f").schema();

      for (int i = 0; i < getBatchSize(); i++) {
        final GenericRecord rec = new GenericData.Record(this.schema);
        final int tag = super.getRandom().nextInt(2);

        rec.put("f", GenericData.get().createEnum(enumSchema.getEnumSymbols().get(tag), enumSchema));

        writer.write(rec, encoder);
      }

      this.testData = baos.toByteArray();
    }

    private String mkSchema(String subschema) {
      return ("{ \"type\": \"record\", \"name\": \"R\", \"fields\": [\n" + "{ \"name\": \"f\", \"type\": " + subschema
          + "}\n" + "] }");
    }

    @Setup(Level.Invocation)
    public void doSetupInvocation() throws Exception {
      this.decoder = super.newDecoder(this.testData);
    }
  }
}
