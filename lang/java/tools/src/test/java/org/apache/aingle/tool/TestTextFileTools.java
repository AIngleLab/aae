/*

 */
package org.apache.aingle.tool;

import static org.junit.Assert.assertEquals;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Random;
import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.generic.GenericDatumReader;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.ClassRule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

@SuppressWarnings("deprecation")
public class TestTextFileTools {
  private static final int COUNT = Integer.parseInt(System.getProperty("test.count", "10"));

  private static final byte[] LINE_SEP = System.getProperty("line.separator").getBytes(StandardCharsets.UTF_8);
  private static File linesFile;
  private static ByteBuffer[] lines;
  static Schema schema;

  @ClassRule
  public static TemporaryFolder DIR = new TemporaryFolder();

  @BeforeClass
  public static void writeRandomFile() throws IOException {
    schema = Schema.create(Type.BYTES);
    lines = new ByteBuffer[COUNT];
    linesFile = new File(DIR.getRoot(), "random.lines");

    OutputStream out = new BufferedOutputStream(new FileOutputStream(linesFile));
    Random rand = new Random();
    for (int j = 0; j < COUNT; j++) {
      byte[] line = new byte[rand.nextInt(512)];
      System.out.println("Creating line = " + line.length);
      for (int i = 0; i < line.length; i++) {
        int b = rand.nextInt(256);
        while (b == '\n' || b == '\r')
          b = rand.nextInt(256);
        line[i] = (byte) b;
      }
      out.write(line);
      out.write(LINE_SEP);
      lines[j] = ByteBuffer.wrap(line);
    }
    out.close();
  }

  private void fromText(String name, String... args) throws Exception {
    File aingleFile = new File(DIR.getRoot(), name + ".aingle");

    ArrayList<String> arglist = new ArrayList<>(Arrays.asList(args));
    arglist.add(linesFile.toString());
    arglist.add(aingleFile.toString());

    new FromTextTool().run(null, null, null, arglist);

    // Read it back, and make sure it's valid.
    DataFileReader<ByteBuffer> file = new DataFileReader<>(aingleFile, new GenericDatumReader<>());
    int i = 0;
    for (ByteBuffer line : file) {
      System.out.println("Reading line = " + line.remaining());
      assertEquals(line, lines[i]);
      i++;
    }
    assertEquals(COUNT, i);
    file.close();
  }

  @Test
  public void testFromText() throws Exception {
    fromText("null", "--codec", "null");
    fromText("deflate", "--codec", "deflate");
    fromText("snappy", "--codec", "snappy");
  }

  @AfterClass
  public static void testToText() throws Exception {
    toText("null");
    toText("deflate");
    toText("snappy");
  }

  private static void toText(String name) throws Exception {
    File aingleFile = new File(DIR.getRoot(), name + ".aingle");
    File outFile = new File(DIR.getRoot(), name + ".lines");

    ArrayList<String> arglist = new ArrayList<>();
    arglist.add(aingleFile.toString());
    arglist.add(outFile.toString());

    new ToTextTool().run(null, null, null, arglist);

    // Read it back, and make sure it's valid.
    try (InputStream orig = new BufferedInputStream(new FileInputStream(linesFile))) {
      try (InputStream after = new BufferedInputStream(new FileInputStream(outFile))) {
        int b;
        while ((b = orig.read()) != -1) {
          assertEquals(b, after.read());
        }
        assertEquals(-1, after.read());
      }
    }
  }

  @Test
  public void testDefaultCodec() throws Exception {
    // The default codec for fromtext is deflate
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    PrintStream err = new PrintStream(baos);
    new FromTextTool().run(null, null, err, Collections.emptyList());
    Assert.assertTrue(baos.toString().contains("Compression codec (default: deflate)"));
  }
}
