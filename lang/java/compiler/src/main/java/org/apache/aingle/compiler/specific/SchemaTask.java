/*

 */
package org.apache.aingle.compiler.specific;

import java.io.File;
import java.io.IOException;

import org.apache.aingle.Schema;

/** Ant task to generate Java interface and classes for a protocol. */
public class SchemaTask extends ProtocolTask {
  @Override
  protected void doCompile(File src, File dest) throws IOException {
    final Schema.Parser parser = new Schema.Parser();
    final Schema schema = parser.parse(src);
    final SpecificCompiler compiler = new SpecificCompiler(schema);
    compiler.setStringType(getStringType());
    compiler.compileToDestination(src, dest);
  }

  public static void main(String[] args) throws IOException {
    if (args.length < 2) {
      System.err.println("Usage: SchemaTask <schema.ain>... <output-folder>");
      System.exit(1);
    }
    File dst = new File(args[args.length - 1]);
    for (int i = 0; i < args.length - 1; i++)
      new SchemaTask().doCompile(new File(args[i]), dst);
  }
}
