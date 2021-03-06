/*

 */
package org.apache.aingle.tool;

import org.apache.aingle.Schema;
import org.apache.aingle.Schema.Type;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumWriter;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import org.junit.rules.TestName;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

import static org.junit.Assert.assertEquals;

public class TestRecordCountTool {

  @Rule
  public TestName name = new TestName();
  @Rule
  public TemporaryFolder temporaryFolder = new TemporaryFolder();

  private File generateData(int numRecords) throws Exception {
    final File tempFile = temporaryFolder.newFile();

    Schema schema = Schema.create(Type.STRING);
    try (DataFileWriter<Object> writer = new DataFileWriter<>(new GenericDatumWriter<>(schema))) {
      writer.create(schema, tempFile);

      // ~10 records per block
      writer.setSyncInterval(60);
      for (int i = 0; i < numRecords; i++) {
        writer.append("foobar");
      }
    }
    return tempFile;
  }

  @Test(expected = FileNotFoundException.class)
  public void testFileDoesNotExist() throws Exception {
    List<String> args = Collections
        .singletonList(new File(temporaryFolder.getRoot(), "nonExistingFile").getAbsolutePath());
    int returnCode = new RecordCountTool().run(System.in, System.out, System.err, args);
    assertEquals(1, returnCode);
  }

  @Test
  public void testBasic() throws Exception {
    final List<Integer> inputSizes = IntStream.range(0, 20).boxed().collect(Collectors.toList());
    for (Integer inputSize : inputSizes) {
      File inputFile = generateData(inputSize);
      List<String> args = Collections.singletonList(inputFile.getAbsolutePath());
      final ByteArrayOutputStream out = new ByteArrayOutputStream();
      int returnCode = new RecordCountTool().run(System.in, new PrintStream(out), System.err, args);

      assertEquals(0, returnCode);
      assertEquals(inputSize.toString() + System.lineSeparator(), out.toString());
    }
  }

  @Test
  public void testMultipleFiles() throws Exception {
    File f1 = generateData(20);
    File f2 = generateData(200);

    List<String> args = Arrays.asList(f1.getAbsolutePath(), f2.getAbsolutePath());
    final ByteArrayOutputStream out = new ByteArrayOutputStream();
    int returnCode = new RecordCountTool().run(System.in, new PrintStream(out), System.err, args);

    assertEquals(0, returnCode);
    assertEquals("220" + System.lineSeparator(), out.toString());
  }

}
