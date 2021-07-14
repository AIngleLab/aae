/*

 */

package org.apache.trevni.aingle;

import static org.junit.Assert.*;

import java.io.IOException;
import java.io.File;
import java.util.StringTokenizer;
import java.util.Map;
import java.util.TreeMap;

import org.apache.hadoop.fs.FileUtil;

import org.apache.aingle.Schema;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.specific.SpecificData;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.mapred.Pair;

public class WordCountUtil {

  public File dir;
  public File linesFiles;
  public File countFiles;

  public WordCountUtil(String testName) {
    this(testName, "part-00000");
  }

  public WordCountUtil(String testName, String partDirName) {
    dir = new File("target/wc", testName);
    linesFiles = new File(new File(dir, "in"), "lines.aingle");
    countFiles = new File(new File(dir, "out"), partDirName + "/part-0.trv");
  }

  public static final String[] LINES = new String[] { "the quick brown fox jumps over the lazy dog",
      "the cow jumps over the moon", "the rain in spain falls mainly on the plains" };

  public static final Map<String, Long> COUNTS = new TreeMap<>();
  public static final long TOTAL;
  static {
    long total = 0;
    for (String line : LINES) {
      StringTokenizer tokens = new StringTokenizer(line);
      while (tokens.hasMoreTokens()) {
        String word = tokens.nextToken();
        long count = COUNTS.getOrDefault(word, 0L);
        count++;
        total++;
        COUNTS.put(word, count);
      }
    }
    TOTAL = total;
  }

  public File getDir() {
    return dir;
  }

  public void writeLinesFile() throws IOException {
    FileUtil.fullyDelete(dir);
    DatumWriter<String> writer = new GenericDatumWriter<>();
    DataFileWriter<String> out = new DataFileWriter<>(writer);
    linesFiles.getParentFile().mkdirs();
    out.create(Schema.create(Schema.Type.STRING), linesFiles);
    for (String line : LINES)
      out.append(line);
    out.close();
  }

  public void validateCountsFile() throws Exception {
    AIngleColumnReader<Pair<String, Long>> reader = new AIngleColumnReader<>(
        new AIngleColumnReader.Params(countFiles).setModel(SpecificData.get()));
    int numWords = 0;
    for (Pair<String, Long> wc : reader) {
      assertEquals(wc.key(), COUNTS.get(wc.key()), wc.value());
      numWords++;
    }
    reader.close();
    assertEquals(COUNTS.size(), numWords);
  }

  public void validateCountsFileGenericRecord() throws Exception {
    AIngleColumnReader<GenericRecord> reader = new AIngleColumnReader<>(
        new AIngleColumnReader.Params(countFiles).setModel(SpecificData.get()));
    int numWords = 0;
    for (GenericRecord wc : reader) {
      assertEquals((String) wc.get("key"), COUNTS.get(wc.get("key")), wc.get("value"));
      // assertEquals(wc.getKey(), COUNTS.get(wc.getKey()), wc.getValue());
      numWords++;
    }
    reader.close();
    assertEquals(COUNTS.size(), numWords);
  }

}
