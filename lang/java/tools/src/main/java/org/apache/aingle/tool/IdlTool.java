/*

 */

package org.apache.aingle.tool;

import org.apache.aingle.Protocol;
import org.apache.aingle.compiler.idl.Idl;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.List;

/**
 * Tool implementation for generating AIngle JSON schemata from idl format files.
 */
public class IdlTool implements Tool {
  @Override
  public int run(InputStream in, PrintStream out, PrintStream err, List<String> args) throws Exception {

    PrintStream parseOut = out;

    if (args.size() > 2 || (args.size() == 1 && (args.get(0).equals("--help") || args.get(0).equals("-help")))) {
      err.println("Usage: idl [in] [out]");
      err.println();
      err.println("If an output path is not specified, outputs to stdout.");
      err.println("If no input or output is specified, takes input from");
      err.println("stdin and outputs to stdin.");
      err.println("The special path \"-\" may also be specified to refer to");
      err.println("stdin and stdout.");
      return -1;
    }

    Idl parser;
    if (args.size() >= 1 && !"-".equals(args.get(0))) {
      parser = new Idl(new File(args.get(0)));
    } else {
      parser = new Idl(in);
    }

    if (args.size() == 2 && !"-".equals(args.get(1))) {
      parseOut = new PrintStream(new FileOutputStream(args.get(1)));
    }

    Protocol p = parser.CompilationUnit();
    try {
      parseOut.print(p.toString(true));
    } finally {
      if (parseOut != out) // Close only the newly created FileOutputStream
        parseOut.close();
    }
    return 0;
  }

  @Override
  public String getName() {
    return "idl";
  }

  @Override
  public String getShortDescription() {
    return "Generates a JSON schema from an AIngle IDL file";
  }
}
