/*

 */
package org.apache.aingle.tool;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;

import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.io.JsonEncoder;

/** Reads a data file and dumps to JSON */
public class DataFileReadTool implements Tool {
  private static final long DEFAULT_HEAD_COUNT = 10;

  @Override
  public String getName() {
    return "tojson";
  }

  @Override
  public String getShortDescription() {
    return "Dumps an AIngle data file as JSON, record per line or pretty.";
  }

  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {
    OptionParser optionParser = new OptionParser();
    OptionSpec<Void> prettyOption = optionParser.accepts("pretty", "Turns on pretty printing.");
    String headDesc = String.format("Converts the first X records (default is %d).", DEFAULT_HEAD_COUNT);
    OptionSpec<String> headOption = optionParser.accepts("head", headDesc).withOptionalArg();
    OptionSpec<String> readerSchemaFileOption = optionParser.accepts("reader-schema-file", "Reader schema file")
        .withOptionalArg().ofType(String.class);
    OptionSpec<String> readerSchemaOption = optionParser.accepts("reader-schema", "Reader schema").withOptionalArg()
        .ofType(String.class);

    OptionSet optionSet = optionParser.parse(args.toArray(new String[0]));
    Boolean pretty = optionSet.has(prettyOption);
    List<String> nargs = new ArrayList<>((List<String>) optionSet.nonOptionArguments());

    String readerSchemaStr = readerSchemaOption.value(optionSet);
    String readerSchemaFile = readerSchemaFileOption.value(optionSet);

    Schema readerSchema = null;
    if (readerSchemaFile != null) {
      readerSchema = Util.parseSchemaFromFS(readerSchemaFile);
    } else if (readerSchemaStr != null) {
      readerSchema = new Schema.Parser().parse(readerSchemaStr);
    }

    long headCount = getHeadCount(optionSet, headOption, nargs);

    if (nargs.size() != 1) {
      printHelp(err);
      err.println();
      optionParser.printHelpOn(err);
      return 1;
    }

    BufferedInputStream inStream = Util.fileOrStdin(nargs.get(0), stdin);

    GenericDatumReader<Object> reader = new GenericDatumReader<>();
    if (readerSchema != null) {
      reader.setExpected(readerSchema);
    }
    try (DataFileStream<Object> streamReader = new DataFileStream<>(inStream, reader)) {
      Schema schema = readerSchema != null ? readerSchema : streamReader.getSchema();
      DatumWriter writer = new GenericDatumWriter<>(schema);
      JsonEncoder encoder = EncoderFactory.get().jsonEncoder(schema, out, pretty);
      for (long recordCount = 0; streamReader.hasNext() && recordCount < headCount; recordCount++) {
        Object datum = streamReader.next();
        writer.write(datum, encoder);
      }
      encoder.flush();
      out.println();
      out.flush();
    }
    return 0;
  }

  private static long getHeadCount(OptionSet optionSet, OptionSpec<String> headOption, List<String> nargs) {
    long headCount = Long.MAX_VALUE;
    if (optionSet.has(headOption)) {
      headCount = DEFAULT_HEAD_COUNT;
      List<String> headValues = optionSet.valuesOf(headOption);
      if (headValues.size() > 0) {
        // if the value parses to int, assume it's meant to go with --head
        // otherwise assume it was an optionSet.nonOptionArgument and add back to the
        // list
        // TODO: support input filenames whose whole path+name is int parsable?
        try {
          headCount = Long.parseLong(headValues.get(0));
          if (headCount < 0)
            throw new AIngleRuntimeException("--head count must not be negative");
        } catch (NumberFormatException ex) {
          nargs.addAll(headValues);
        }
      }
    }
    return headCount;
  }

  private void printHelp(PrintStream ps) {
    ps.println("tojson [--pretty] [--head[=X]] input-file");
    ps.println();
    ps.println(getShortDescription());
    ps.println("A dash ('-') can be given as an input file to use stdin");
  }
}
