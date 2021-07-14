/*

 */
package org.apache.aingle.tool;

import static java.util.Arrays.asList;

import java.io.File;
import java.io.FileInputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collections;

import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestRecodecTool {
  @Rule
  public TemporaryFolder DIR = new TemporaryFolder();

  @Test
  public void testRecodec() throws Exception {
    String metaKey = "myMetaKey";
    String metaValue = "myMetaValue";

    File inputFile = new File(DIR.getRoot(), "input.aingle");

    Schema schema = Schema.create(Type.STRING);
    DataFileWriter<String> writer = new DataFileWriter<>(new GenericDatumWriter<String>(schema));
    writer.setMeta(metaKey, metaValue).create(schema, inputFile);
    // We write some garbage which should be quite compressible by deflate,
    // but is complicated enough that deflate-9 will work better than deflate-1.
    // These values were plucked from thin air and worked on the first try, so
    // don't read too much into them.
    for (int i = 0; i < 100000; i++) {
      writer.append("" + i % 100);
    }
    writer.close();

    File defaultOutputFile = new File(DIR.getRoot(), "default-output.aingle");
    File nullOutputFile = new File(DIR.getRoot(), "null-output.aingle");
    File deflateDefaultOutputFile = new File(DIR.getRoot(), "deflate-default-output.aingle");
    File deflate1OutputFile = new File(DIR.getRoot(), "deflate-1-output.aingle");
    File deflate9OutputFile = new File(DIR.getRoot(), "deflate-9-output.aingle");

    new RecodecTool().run(new FileInputStream(inputFile), new PrintStream(defaultOutputFile), null, new ArrayList<>());
    new RecodecTool().run(new FileInputStream(inputFile), new PrintStream(nullOutputFile), null,
        Collections.singletonList("--codec=null"));
    new RecodecTool().run(new FileInputStream(inputFile), new PrintStream(deflateDefaultOutputFile), null,
        Collections.singletonList("--codec=deflate"));
    new RecodecTool().run(new FileInputStream(inputFile), new PrintStream(deflate1OutputFile), null,
        asList("--codec=deflate", "--level=1"));
    new RecodecTool().run(new FileInputStream(inputFile), new PrintStream(deflate9OutputFile), null,
        asList("--codec=deflate", "--level=9"));

    // We assume that metadata copying is orthogonal to codec selection, and
    // so only test it for a single file.
    try (DataFileReader<Void> reader = new DataFileReader<Void>(defaultOutputFile, new GenericDatumReader<>())) {
      Assert.assertEquals(metaValue, reader.getMetaString(metaKey));
    }

    // The "default" codec should be the same as null.
    Assert.assertEquals(defaultOutputFile.length(), nullOutputFile.length());

    // All of the deflated files should be smaller than the null file.
    assertLessThan(deflateDefaultOutputFile.length(), nullOutputFile.length());
    assertLessThan(deflate1OutputFile.length(), nullOutputFile.length());
    assertLessThan(deflate9OutputFile.length(), nullOutputFile.length());

    // The "level 9" file should be smaller than the "level 1" file.
    assertLessThan(deflate9OutputFile.length(), deflate1OutputFile.length());
  }

  private static void assertLessThan(long less, long more) {
    if (less >= more) {
      Assert.fail("Expected " + less + " to be less than " + more);
    }
  }
}
