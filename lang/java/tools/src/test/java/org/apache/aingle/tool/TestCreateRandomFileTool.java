/*

 */
package org.apache.aingle.tool;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Iterator;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.util.RandomData;
import org.apache.trevni.TestUtil;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class TestCreateRandomFileTool {
  private static final String COUNT = System.getProperty("test.count", "200");
  private static final File DIR = new File("/tmp");
  private static final File OUT_FILE = new File(DIR, "random.aingle");
  private static final File SCHEMA_FILE = new File("../../../share/test/schemas/weather.ain");

  private final Schema.Parser schemaParser = new Schema.Parser();

  private static final long SEED = System.currentTimeMillis();

  private ByteArrayOutputStream out;
  private ByteArrayOutputStream err;

  @Before
  public void before() {
    out = new ByteArrayOutputStream();
    err = new ByteArrayOutputStream();
  }

  @After
  public void after() throws Exception {
    out.close();
    err.close();
  }

  private int run(List<String> args) throws Exception {
    PrintStream output = new PrintStream(out);
    PrintStream saveOut = System.out;
    PrintStream error = new PrintStream(err);
    PrintStream saveErr = System.err;
    try {
      System.setOut(output);
      System.setErr(error);
      return new CreateRandomFileTool().run(null, output, error, args);
    } finally {
      System.setOut(saveOut);
      System.setErr(saveErr);
    }
  }

  private void check(String... extraArgs) throws Exception {
    ArrayList<String> args = new ArrayList<>();
    args.addAll(Arrays.asList(OUT_FILE.toString(), "--count", COUNT, "--schema-file", SCHEMA_FILE.toString(), "--seed",
        Long.toString(SEED)));
    args.addAll(Arrays.asList(extraArgs));
    run(args);

    DataFileReader<Object> reader = new DataFileReader<>(OUT_FILE, new GenericDatumReader<>());

    Iterator<Object> found = reader.iterator();
    for (Object expected : new RandomData(schemaParser.parse(SCHEMA_FILE), Integer.parseInt(COUNT), SEED))
      assertEquals(expected, found.next());

    reader.close();
  }

  private void checkMissingCount(String... extraArgs) throws Exception {
    ArrayList<String> args = new ArrayList<>();
    args.addAll(
        Arrays.asList(OUT_FILE.toString(), "--schema-file", SCHEMA_FILE.toString(), "--seed", Long.toString(SEED)));
    args.addAll(Arrays.asList(extraArgs));
    run(args);
    assertTrue(err.toString().contains("Need count (--count)"));
  }

  @Test
  public void testSimple() throws Exception {
    check();
  }

  @Test
  public void testCodec() throws Exception {
    check("--codec", "snappy");
  }

  @Test
  public void testMissingCountParameter() throws Exception {
    checkMissingCount();
  }

  @Test
  public void testStdOut() throws Exception {
    TestUtil.resetRandomSeed();
    run(Arrays.asList("-", "--count", COUNT, "--schema-file", SCHEMA_FILE.toString(), "--seed", Long.toString(SEED)));

    byte[] file = out.toByteArray();

    DataFileStream<Object> reader = new DataFileStream<>(new ByteArrayInputStream(file), new GenericDatumReader<>());

    Iterator<Object> found = reader.iterator();
    for (Object expected : new RandomData(schemaParser.parse(SCHEMA_FILE), Integer.parseInt(COUNT), SEED))
      assertEquals(expected, found.next());

    reader.close();
  }

  @Test
  public void testDefaultCodec() throws Exception {
    // The default codec for random is deflate
    run(Collections.emptyList());
    assertTrue(err.toString().contains("Compression codec (default: deflate)"));
  }
}
