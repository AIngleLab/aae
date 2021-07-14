/*

 */
package org.apache.aingle.tool;

import static org.junit.Assert.assertEquals;

import java.io.File;
import java.util.Arrays;
import java.util.List;

import org.junit.Test;

public class TestIdlToSchemataTool {

  @Test
  public void testSplitIdlIntoSchemata() throws Exception {
    String idl = "src/test/idl/protocol.avdl";
    String outdir = "target/test-split";

    List<String> arglist = Arrays.asList(idl, outdir);
    new IdlToSchemataTool().run(null, null, null, arglist);

    String[] files = new File(outdir).list();
    assertEquals(4, files.length);
  }
}
