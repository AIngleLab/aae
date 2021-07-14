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
import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.io.JsonEncoder;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;

/** Converts an input file from AIngle binary into JSON. */
public class BinaryFragmentToJsonTool implements Tool {
  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {
    OptionParser optionParser = new OptionParser();
    OptionSpec<Void> noPrettyOption = optionParser.accepts("no-pretty", "Turns off pretty printing.");
    OptionSpec<String> schemaFileOption = optionParser
        .accepts("schema-file", "File containing schema, must not occur with inline schema.").withOptionalArg()
        .ofType(String.class);

    OptionSet optionSet = optionParser.parse(args.toArray(new String[0]));
    Boolean noPretty = optionSet.has(noPrettyOption);
    List<String> nargs = (List<String>) optionSet.nonOptionArguments();
    String schemaFile = schemaFileOption.value(optionSet);

    if (nargs.size() != (schemaFile == null ? 2 : 1)) {
      err.println("fragtojson --no-pretty --schema-file <file> [inline-schema] input-file");
      err.println("   converts AIngle fragments to JSON.");
      optionParser.printHelpOn(err);
      err.println("   A dash '-' for input-file means stdin.");
      return 1;
    }
    Schema schema;
    String inputFile;
    if (schemaFile == null) {
      schema = new Schema.Parser().parse(nargs.get(0));
      inputFile = nargs.get(1);
    } else {
      schema = Util.parseSchemaFromFS(schemaFile);
      inputFile = nargs.get(0);
    }
    InputStream input = Util.fileOrStdin(inputFile, stdin);

    try {
      DatumReader<Object> reader = new GenericDatumReader<>(schema);
      BinaryDecoder binaryDecoder = DecoderFactory.get().binaryDecoder(input, null);
      DatumWriter<Object> writer = new GenericDatumWriter<>(schema);
      JsonEncoder jsonEncoder = EncoderFactory.get().jsonEncoder(schema, out, !noPretty);
      Object datum = null;
      while (!binaryDecoder.isEnd()) {
        datum = reader.read(datum, binaryDecoder);
        writer.write(datum, jsonEncoder);
        jsonEncoder.flush();
      }
      out.println();
      out.flush();
    } finally {
      Util.close(input);
    }
    return 0;
  }

  @Override
  public String getName() {
    return "fragtojson";
  }

  @Override
  public String getShortDescription() {
    return "Renders a binary-encoded AIngle datum as JSON.";
  }
}
