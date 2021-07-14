/*

 */
package org.apache.aingle.tool;

import java.io.DataInputStream;
import java.io.EOFException;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.List;

import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileConstants;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.DecoderFactory;

/** Reads new-line delimited JSON records and writers an AIngle data file. */
public class DataFileWriteTool implements Tool {

  @Override
  public String getName() {
    return "fromjson";
  }

  @Override
  public String getShortDescription() {
    return "Reads JSON records and writes an AIngle data file.";
  }

  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {

    OptionParser p = new OptionParser();
    OptionSpec<String> codec = Util.compressionCodecOptionWithDefault(p, DataFileConstants.NULL_CODEC);
    OptionSpec<Integer> level = Util.compressionLevelOption(p);
    OptionSpec<String> file = p.accepts("schema-file", "Schema File").withOptionalArg().ofType(String.class);
    OptionSpec<String> inschema = p.accepts("schema", "Schema").withOptionalArg().ofType(String.class);
    OptionSet opts = p.parse(args.toArray(new String[0]));

    List<String> nargs = (List<String>) opts.nonOptionArguments();
    if (nargs.size() != 1) {
      err.println("Expected 1 arg: input_file");
      p.printHelpOn(err);
      return 1;
    }
    String schemastr = inschema.value(opts);
    String schemafile = file.value(opts);
    if (schemastr == null && schemafile == null) {
      err.println("Need an input schema file (--schema-file) or inline schema (--schema)");
      p.printHelpOn(err);
      return 1;
    }
    Schema schema = (schemafile != null) ? Util.parseSchemaFromFS(schemafile) : new Schema.Parser().parse(schemastr);

    DatumReader<Object> reader = new GenericDatumReader<>(schema);

    InputStream input = Util.fileOrStdin(nargs.get(0), stdin);
    try {
      DataInputStream din = new DataInputStream(input);
      DataFileWriter<Object> writer = new DataFileWriter<>(new GenericDatumWriter<>());
      writer.setCodec(Util.codecFactory(opts, codec, level, DataFileConstants.NULL_CODEC));
      writer.create(schema, out);
      Decoder decoder = DecoderFactory.get().jsonDecoder(schema, din);
      Object datum;
      while (true) {
        try {
          datum = reader.read(null, decoder);
        } catch (EOFException e) {
          break;
        }
        writer.append(datum);
      }
      writer.close();
    } finally {
      Util.close(input);
    }
    return 0;
  }

}
