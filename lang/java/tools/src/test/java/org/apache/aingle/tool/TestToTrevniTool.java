/*

 */
package org.apache.aingle.tool;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.PrintStream;
import java.util.Arrays;
import java.util.Iterator;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.util.RandomData;
import org.apache.trevni.aingle.AIngleColumnReader;
import org.junit.Test;
import static org.junit.Assert.assertEquals;

public class TestToTrevniTool {
  private static final long SEED = System.currentTimeMillis();

  private static final int COUNT = Integer.parseInt(System.getProperty("test.count", "200"));
  private static final File DIR = new File("/tmp");
  private static final File AINGLE_FILE = new File(DIR, "random.aingle");
  private static final File TREVNI_FILE = new File(DIR, "random.trv");
  private static final File SCHEMA_FILE = new File("../../../share/test/schemas/weather.ain");

  private String run(String... args) throws Exception {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    PrintStream p = new PrintStream(baos);
    new ToTrevniTool().run(null, p, null, Arrays.asList(args));
    return baos.toString("UTF-8").replace("\r", "");
  }

  @Test
  public void test() throws Exception {
    Schema schema = new Schema.Parser().parse(SCHEMA_FILE);

    DataFileWriter<Object> writer = new DataFileWriter<>(new GenericDatumWriter<>());
    writer.create(schema, Util.createFromFS(AINGLE_FILE.toString()));
    for (Object datum : new RandomData(schema, COUNT, SEED))
      writer.append(datum);
    writer.close();

    run(AINGLE_FILE.toString(), TREVNI_FILE.toString());

    AIngleColumnReader<Object> reader = new AIngleColumnReader<>(new AIngleColumnReader.Params(TREVNI_FILE));
    Iterator<Object> found = reader.iterator();
    for (Object expected : new RandomData(schema, COUNT, SEED))
      assertEquals(expected, found.next());
    reader.close();
  }

}
