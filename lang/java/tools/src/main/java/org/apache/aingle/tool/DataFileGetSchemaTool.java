/*

 */
package org.apache.aingle.tool;

import java.io.InputStream;
import java.io.PrintStream;
import java.util.List;

import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.generic.GenericDatumReader;

/** Reads a data file to get its schema. */
public class DataFileGetSchemaTool implements Tool {

  @Override
  public String getName() {
    return "getschema";
  }

  @Override
  public String getShortDescription() {
    return "Prints out schema of an AIngle data file.";
  }

  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {
    if (args.size() != 1) {
      err.println("Expected 1 argument: input_file");
      return 1;
    }
    DataFileReader<Void> reader = new DataFileReader<>(Util.openSeekableFromFS(args.get(0)),
        new GenericDatumReader<>());
    out.println(reader.getSchema().toString(true));
    reader.close();
    return 0;
  }
}
