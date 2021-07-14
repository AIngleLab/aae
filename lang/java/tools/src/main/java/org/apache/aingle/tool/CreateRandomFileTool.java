/*

 */
package org.apache.aingle.tool;

import java.io.InputStream;
import java.io.PrintStream;
import java.util.List;

import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.util.RandomData;

/** Creates a file filled with randomly-generated instances of a schema. */
public class CreateRandomFileTool implements Tool {

  @Override
  public String getName() {
    return "random";
  }

  @Override
  public String getShortDescription() {
    return "Creates a file with randomly generated instances of a schema.";
  }

  @SuppressWarnings("unchecked")
  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {

    OptionParser p = new OptionParser();
    OptionSpec<Integer> count = p.accepts("count", "Record Count").withRequiredArg().ofType(Integer.class);
    OptionSpec<String> codec = Util.compressionCodecOption(p);
    OptionSpec<Integer> level = Util.compressionLevelOption(p);
    OptionSpec<String> file = p.accepts("schema-file", "Schema File").withOptionalArg().ofType(String.class);
    OptionSpec<String> inschema = p.accepts("schema", "Schema").withOptionalArg().ofType(String.class);
    OptionSpec<Long> seedOpt = p.accepts("seed", "Seed for random").withOptionalArg().ofType(Long.class);

    OptionSet opts = p.parse(args.toArray(new String[0]));
    if (opts.nonOptionArguments().size() != 1) {
      err.println("Usage: outFile (filename or '-' for stdout)");
      p.printHelpOn(err);
      return 1;
    }
    args = (List<String>) opts.nonOptionArguments();

    String schemastr = inschema.value(opts);
    String schemafile = file.value(opts);
    Long seed = seedOpt.value(opts);
    if (schemastr == null && schemafile == null) {
      err.println("Need input schema (--schema-file) or (--schema)");
      p.printHelpOn(err);
      return 1;
    }
    Schema schema = (schemafile != null) ? Util.parseSchemaFromFS(schemafile) : new Schema.Parser().parse(schemastr);

    DataFileWriter<Object> writer = new DataFileWriter<>(new GenericDatumWriter<>());
    writer.setCodec(Util.codecFactory(opts, codec, level));
    writer.create(schema, Util.fileOrStdout(args.get(0), out));

    Integer countValue = count.value(opts);
    if (countValue == null) {
      err.println("Need count (--count)");
      p.printHelpOn(err);
      writer.close();
      return 1;
    }

    RandomData rd = seed == null ? new RandomData(schema, countValue) : new RandomData(schema, countValue, seed);
    for (Object datum : rd)
      writer.append(datum);

    writer.close();

    return 0;
  }

}
