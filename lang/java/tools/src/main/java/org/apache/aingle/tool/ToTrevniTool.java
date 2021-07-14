/*

 */
package org.apache.aingle.tool;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.List;

import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.generic.GenericDatumReader;

import org.apache.trevni.ColumnFileMetaData;
import org.apache.trevni.aingle.AIngleColumnWriter;

import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;

/** Reads an AIngle data file and writes a Trevni file. */
public class ToTrevniTool implements Tool {

  @Override
  public String getName() {
    return "totrevni";
  }

  @Override
  public String getShortDescription() {
    return "Converts an AIngle data file to a Trevni file.";
  }

  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {

    OptionParser p = new OptionParser();
    OptionSpec<String> codec = p.accepts("codec", "Compression codec").withRequiredArg().defaultsTo("null")
        .ofType(String.class);
    OptionSet opts = p.parse(args.toArray(new String[0]));
    if (opts.nonOptionArguments().size() != 2) {
      err.println("Usage: inFile outFile (filenames or '-' for stdin/stdout)");
      p.printHelpOn(err);
      return 1;
    }
    args = (List<String>) opts.nonOptionArguments();

    DataFileStream<Object> reader = new DataFileStream(Util.fileOrStdin(args.get(0), stdin),
        new GenericDatumReader<>());
    OutputStream outs = Util.fileOrStdout(args.get(1), out);
    AIngleColumnWriter<Object> writer = new AIngleColumnWriter<>(reader.getSchema(),
        new ColumnFileMetaData().setCodec(codec.value(opts)));
    for (Object datum : reader)
      writer.write(datum);
    writer.writeTo(outs);
    outs.close();
    reader.close();
    return 0;
  }

}
