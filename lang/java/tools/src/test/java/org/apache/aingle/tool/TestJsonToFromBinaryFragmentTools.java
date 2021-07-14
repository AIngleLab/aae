/*

 */
package org.apache.aingle.tool;

import static org.junit.Assert.assertEquals;

import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Type;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

/**
 * Tests both {@link JsonToBinaryFragmentTool} and
 * {@link BinaryFragmentToJsonTool}.
 */
public class TestJsonToFromBinaryFragmentTools {
  private static final String STRING_SCHEMA = Schema.create(Type.STRING).toString();
  private static final String UTF8 = "utf-8";
  private static final String AINGLE = "ZLong string implies readable length encoding.";
  private static final String JSON = "\"Long string implies readable length encoding.\"\n";

  @Rule
  public TemporaryFolder DIR = new TemporaryFolder();

  @Test
  public void testBinaryToJson() throws Exception {
    binaryToJson(AINGLE, JSON, STRING_SCHEMA);
  }

  @Test
  public void testJsonToBinary() throws Exception {
    jsonToBinary(JSON, AINGLE, STRING_SCHEMA);
  }

  @Test
  public void testMultiBinaryToJson() throws Exception {
    binaryToJson(AINGLE + AINGLE + AINGLE, JSON + JSON + JSON, STRING_SCHEMA);
  }

  @Test
  public void testMultiJsonToBinary() throws Exception {
    jsonToBinary(JSON + JSON + JSON, AINGLE + AINGLE + AINGLE, STRING_SCHEMA);
  }

  @Test
  public void testBinaryToNoPrettyJson() throws Exception {
    binaryToJson(AINGLE, JSON, "--no-pretty", STRING_SCHEMA);
  }

  @Test
  public void testMultiBinaryToNoPrettyJson() throws Exception {
    binaryToJson(AINGLE + AINGLE + AINGLE, JSON + JSON + JSON, "--no-pretty", STRING_SCHEMA);
  }

  @Test
  public void testBinaryToJsonSchemaFile() throws Exception {
    binaryToJson(AINGLE, JSON, "--schema-file", schemaFile(DIR.getRoot()));
  }

  @Test
  public void testJsonToBinarySchemaFile() throws Exception {
    jsonToBinary(JSON, AINGLE, "--schema-file", schemaFile(DIR.getRoot()));
  }

  private void binaryToJson(String aingle, String json, String... options) throws Exception {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    PrintStream p = new PrintStream(new BufferedOutputStream(baos));

    List<String> args = new ArrayList<>(Arrays.asList(options));
    args.add("-");
    new BinaryFragmentToJsonTool().run(new ByteArrayInputStream(aingle.getBytes(StandardCharsets.UTF_8)), // stdin
        p, // stdout
        null, // stderr
        args);
    System.out.println(baos.toString(UTF8).replace("\r", ""));
    assertEquals(json, baos.toString(UTF8).replace("\r", ""));
  }

  private void jsonToBinary(String json, String aingle, String... options) throws Exception {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    PrintStream p = new PrintStream(new BufferedOutputStream(baos));

    List<String> args = new ArrayList<>(Arrays.asList(options));
    args.add("-");
    new JsonToBinaryFragmentTool().run(new ByteArrayInputStream(json.getBytes(StandardCharsets.UTF_8)), // stdin
        p, // stdout
        null, // stderr
        args);
    assertEquals(aingle, baos.toString(UTF8));
  }

  private static String schemaFile(File dir) throws IOException {
    File schemaFile = new File(dir, "String.ain");
    try (FileWriter fw = new FileWriter(schemaFile)) {
      fw.append(STRING_SCHEMA);
    }
    return schemaFile.toString();
  }
}
