/*

 */
package org.apache.aingle.tool;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.List;

import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;

import org.apache.aingle.Schema;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumWriter;

/**
 * Reads a text file into an AIngle data file.
 *
 * Can accept a file name, and HDFS file URI, or stdin. Can write to a file
 * name, an HDFS URI, or stdout.
 */
public class FromTextTool implements Tool {
  private static final String TEXT_FILE_SCHEMA = "\"bytes\"";

  @Override
  public String getName() {
    return "fromtext";
  }

  @Override
  public String getShortDescription() {
    return "Imports a text file into an aingle data file.";
  }

  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {

    OptionParser p = new OptionParser();
    OptionSpec<Integer> level = Util.compressionLevelOption(p);
    OptionSpec<String> codec = Util.compressionCodecOption(p);

    OptionSet opts = p.parse(args.toArray(new String[0]));

    List<String> nargs = (List<String>) opts.nonOptionArguments();
    if (nargs.size() != 2) {
      err.println("Expected 2 args: from_file to_file (local filenames," + " Hadoop URI's, or '-' for stdin/stdout");
      p.printHelpOn(err);
      return 1;
    }

    CodecFactory codecFactory = Util.codecFactory(opts, codec, level);

    BufferedInputStream inStream = Util.fileOrStdin(nargs.get(0), stdin);
    BufferedOutputStream outStream = Util.fileOrStdout(nargs.get(1), out);

    DataFileWriter<ByteBuffer> writer = new DataFileWriter<>(new GenericDatumWriter<>());
    writer.setCodec(codecFactory);
    writer.create(new Schema.Parser().parse(TEXT_FILE_SCHEMA), outStream);

    ByteBuffer line = ByteBuffer.allocate(128);
    boolean returnSeen = false;
    byte[] buf = new byte[8192];
    for (int end = inStream.read(buf); end != -1; end = inStream.read(buf)) {
      for (int i = 0; i < end; i++) {
        int b = buf[i] & 0xFF;
        if (b == '\n') { // newline
          if (!returnSeen) {
            System.out.println("Writing line = " + line.position());
            ((Buffer) line).flip();
            writer.append(line);
            ((Buffer) line).clear();
          } else {
            returnSeen = false;
          }
        } else if (b == '\r') { // return
          ((Buffer) line).flip();
          writer.append(line);
          ((Buffer) line).clear();
          returnSeen = true;
        } else {
          if (line.position() == line.limit()) { // reallocate longer line
            ByteBuffer tempLine = ByteBuffer.allocate(line.limit() * 2);
            ((Buffer) line).flip();
            tempLine.put(line);
            line = tempLine;
          }
          line.put((byte) b);
          returnSeen = false;
        }
      }
    }
    writer.close();
    inStream.close();
    return 0;
  }

}
