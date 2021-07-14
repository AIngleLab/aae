/*

 */
package org.apache.aingle.tool;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.Arrays;
import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileConstants;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.BinaryData;
import org.apache.aingle.util.Utf8;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.ClassRule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestDataFileRepairTool {

  @ClassRule
  public static TemporaryFolder DIR = new TemporaryFolder();

  private static final Schema SCHEMA = Schema.create(Schema.Type.STRING);
  private static File corruptBlockFile;
  private static File corruptRecordFile;

  private File repairedFile;

  @BeforeClass
  public static void writeCorruptFile() throws IOException {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    long pos;
    // Write a data file
    try (DataFileWriter<Utf8> w = new DataFileWriter<>(new GenericDatumWriter<>(SCHEMA))) {
      w.create(SCHEMA, baos);
      w.append(new Utf8("apple"));
      w.append(new Utf8("banana"));
      w.append(new Utf8("celery"));
      w.sync();
      w.append(new Utf8("date"));
      w.append(new Utf8("endive"));
      w.append(new Utf8("fig"));
      pos = w.sync();
      w.append(new Utf8("guava"));
      w.append(new Utf8("hazelnut"));
    }

    byte[] original = baos.toByteArray();

    // Corrupt the second block by inserting some zero bytes before the sync marker
    int corruptPosition = (int) pos - DataFileConstants.SYNC_SIZE;
    int corruptedBytes = 3;
    byte[] corrupted = new byte[original.length + corruptedBytes];
    System.arraycopy(original, 0, corrupted, 0, corruptPosition);
    System.arraycopy(original, corruptPosition, corrupted, corruptPosition + corruptedBytes,
        original.length - corruptPosition);

    corruptBlockFile = new File(DIR.getRoot(), "corruptBlock.aingle");
    corruptBlockFile.deleteOnExit();
    try (FileOutputStream out = new FileOutputStream(corruptBlockFile)) {
      out.write(corrupted);
    }

    // Corrupt the "endive" record by changing the length of the string to be
    // negative
    corruptPosition = (int) pos - DataFileConstants.SYNC_SIZE - (1 + "fig".length() + 1 + "endive".length());
    corrupted = new byte[original.length];
    System.arraycopy(original, 0, corrupted, 0, original.length);
    BinaryData.encodeLong(-1, corrupted, corruptPosition);

    corruptRecordFile = new File(DIR.getRoot(), "corruptRecord.aingle");
    corruptRecordFile.deleteOnExit();
    try (FileOutputStream out = new FileOutputStream(corruptRecordFile)) {
      out.write(corrupted);
    }
  }

  @Before
  public void setUp() {
    repairedFile = new File(DIR.getRoot(), "repaired.aingle");
  }

  private String run(Tool tool, String... args) throws Exception {
    return run(tool, null, args);
  }

  private String run(Tool tool, InputStream stdin, String... args) throws Exception {
    ByteArrayOutputStream out = new ByteArrayOutputStream();
    PrintStream stdout = new PrintStream(out);
    tool.run(stdin, stdout, System.err, Arrays.asList(args));
    return out.toString("UTF-8").replace("\r", "");
  }

  @Test
  public void testReportCorruptBlock() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "report", corruptBlockFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 2 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 5 Number of corrupt records: 0"));
  }

  @Test
  public void testReportCorruptRecord() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "report", corruptRecordFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 3 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 8 Number of corrupt records: 2"));
  }

  @Test
  public void testRepairAllCorruptBlock() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "all", corruptBlockFile.getPath(), repairedFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 2 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 5 Number of corrupt records: 0"));
    checkFileContains(repairedFile, "apple", "banana", "celery", "guava", "hazelnut");
  }

  @Test
  public void testRepairAllCorruptRecord() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "all", corruptRecordFile.getPath(), repairedFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 3 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 8 Number of corrupt records: 2"));
    checkFileContains(repairedFile, "apple", "banana", "celery", "date", "guava", "hazelnut");
  }

  @Test
  public void testRepairPriorCorruptBlock() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "prior", corruptBlockFile.getPath(), repairedFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 2 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 5 Number of corrupt records: 0"));
    checkFileContains(repairedFile, "apple", "banana", "celery");
  }

  @Test
  public void testRepairPriorCorruptRecord() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "prior", corruptRecordFile.getPath(), repairedFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 3 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 8 Number of corrupt records: 2"));
    checkFileContains(repairedFile, "apple", "banana", "celery", "date");
  }

  @Test
  public void testRepairAfterCorruptBlock() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "after", corruptBlockFile.getPath(), repairedFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 2 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 5 Number of corrupt records: 0"));
    checkFileContains(repairedFile, "guava", "hazelnut");
  }

  @Test
  public void testRepairAfterCorruptRecord() throws Exception {
    String output = run(new DataFileRepairTool(), "-o", "after", corruptRecordFile.getPath(), repairedFile.getPath());
    assertTrue(output, output.contains("Number of blocks: 3 Number of corrupt blocks: 1"));
    assertTrue(output, output.contains("Number of records: 8 Number of corrupt records: 2"));
    checkFileContains(repairedFile, "guava", "hazelnut");
  }

  private void checkFileContains(File repairedFile, String... lines) throws IOException {
    DataFileReader r = new DataFileReader<>(repairedFile, new GenericDatumReader<>(SCHEMA));
    for (String line : lines) {
      assertEquals(line, r.next().toString());
    }
    assertFalse(r.hasNext());
    r.close();
  }

}
