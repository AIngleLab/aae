/*

 */
package org.apache.aingle.compiler.specific;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.Protocol;
import org.apache.aingle.generic.GenericData.StringType;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.DirectoryScanner;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.types.FileSet;

/** Ant task to generate Java interface and classes for a protocol. */
public class ProtocolTask extends Task {
  private File src;
  private File dest = new File(".");
  private StringType stringType = StringType.CharSequence;

  private final ArrayList<FileSet> filesets = new ArrayList<>();

  /** Set the schema file. */
  public void setFile(File file) {
    this.src = file;
  }

  /** Set the output directory */
  public void setDestdir(File dir) {
    this.dest = dir;
  }

  /** Set the string type. */
  public void setStringType(StringType type) {
    this.stringType = type;
  }

  /** Get the string type. */
  public StringType getStringType() {
    return this.stringType;
  }

  /** Add a fileset. */
  public void addFileset(FileSet set) {
    filesets.add(set);
  }

  /** Run the compiler. */
  @Override
  public void execute() {
    if (src == null && filesets.size() == 0)
      throw new BuildException("No file or fileset specified.");

    if (src != null)
      compile(src);

    Project myProject = getProject();
    for (FileSet fs : filesets) {
      DirectoryScanner ds = fs.getDirectoryScanner(myProject);
      File dir = fs.getDir(myProject);
      String[] srcs = ds.getIncludedFiles();
      for (String src1 : srcs) {
        compile(new File(dir, src1));
      }
    }
  }

  protected void doCompile(File src, File dir) throws IOException {
    Protocol protocol = Protocol.parse(src);
    SpecificCompiler compiler = new SpecificCompiler(protocol);
    compiler.setStringType(getStringType());
    compiler.compileToDestination(src, dest);
  }

  private void compile(File file) {
    try {
      doCompile(file, dest);
    } catch (AIngleRuntimeException | IOException e) {
      throw new BuildException(e);
    }
  }
}
