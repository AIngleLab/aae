/*

 */
package org.apache.aingle.tool;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.List;

import joptsimple.OptionParser;
import joptsimple.OptionSet;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.generic.GenericDatumReader;

/** Reads an aingle data file into a plain text file. */
public class ToTextTool implements Tool {
  private static final String TEXT_FILE_SCHEMA = "\"bytes\"";
  private static final byte[] LINE_SEPARATOR = System.getProperty("line.separator").getBytes(StandardCharsets.UTF_8);

  @Override
  public String getName() {
    return "totext";
  }

  @Override
  public String getShortDescription() {
    return "Converts an AIngle data file to a text file.";
  }

  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {

    OptionParser p = new OptionParser();
    OptionSet opts = p.parse(args.toArray(new String[0]));
    if (opts.nonOptionArguments().size() != 2) {
      err.println("Expected 2 args: from_file to_file (filenames or '-' for stdin/stdout");
      p.printHelpOn(err);
      return 1;
    }

    BufferedInputStream inStream = Util.fileOrStdin(args.get(0), stdin);
    BufferedOutputStream outStream = Util.fileOrStdout(args.get(1), out);

    GenericDatumReader<Object> reader = new GenericDatumReader<>();
    DataFileStream<Object> fileReader = new DataFileStream<>(inStream, reader);

    if (!fileReader.getSchema().equals(new Schema.Parser().parse(TEXT_FILE_SCHEMA))) {
      err.println("AIngle file is not generic text schema");
      p.printHelpOn(err);
      fileReader.close();
      return 1;
    }

    while (fileReader.hasNext()) {
      ByteBuffer outBuff = (ByteBuffer) fileReader.next();
      outStream.write(outBuff.array());
      outStream.write(LINE_SEPARATOR);
    }
    fileReader.close();
    Util.close(inStream);
    Util.close(outStream);
    return 0;
  }

}
