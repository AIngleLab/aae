/*

 */
package org.apache.aingle.tool;

import java.io.File;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.List;

import org.apache.aingle.Schema;
import org.apache.aingle.util.RandomData;
import org.apache.trevni.ColumnFileMetaData;
import org.apache.trevni.aingle.AIngleColumnWriter;

/** Tool to create randomly populated Trevni file based on an AIngle schema */
public class TrevniCreateRandomTool implements Tool {

  @Override
  public String getName() {
    return "trevni_random";
  }

  @Override
  public String getShortDescription() {
    return "Create a Trevni file filled with random instances of a schema.";
  }

  @Override
  public int run(InputStream stdin, PrintStream out, PrintStream err, List<String> args) throws Exception {
    if (args.size() != 3) {
      err.println("Usage: schemaFile count outputFile");
      return 1;
    }

    File schemaFile = new File(args.get(0));
    int count = Integer.parseInt(args.get(1));
    File outputFile = new File(args.get(2));

    Schema schema = new Schema.Parser().parse(schemaFile);

    AIngleColumnWriter<Object> writer = new AIngleColumnWriter<>(schema, new ColumnFileMetaData());

    for (Object datum : new RandomData(schema, count))
      writer.write(datum);

    writer.writeTo(outputFile);

    return 0;
  }
}
