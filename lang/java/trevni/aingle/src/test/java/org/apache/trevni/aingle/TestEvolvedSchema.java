/*

 */
package org.apache.trevni.aingle;

import java.io.File;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.io.DatumWriter;
import org.apache.trevni.ColumnFileMetaData;
import org.apache.trevni.aingle.AIngleColumnReader.Params;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class TestEvolvedSchema {
  private static String writerSchema = "{" + "    \"namespace\": \"org.apache.aingle\","
      + "    \"name\": \"test_evolution\"," + "    \"type\": \"record\"," + "    \"fields\": ["
      + "        { \"name\": \"a\", \"type\":\"string\" }," + "        { \"name\": \"b\", \"type\":\"int\" }" + "     ]"
      + "}";
  private static String innerSchema = "{\"name\":\"c1\"," + "          \"type\":\"record\","
      + "          \"fields\":[{\"name\":\"c11\", \"type\":\"int\", \"default\": 2},"
      + "                      {\"name\":\"c12\", \"type\":\"string\", \"default\":\"goodbye\"}]}";
  private static String evolvedSchema2 = "{" + "    \"namespace\": \"org.apache.aingle\","
      + "    \"name\": \"test_evolution\"," + "    \"type\": \"record\"," + "    \"fields\": ["
      + "        { \"name\": \"a\", \"type\":\"string\" }," + "        { \"name\": \"b\", \"type\":\"int\" },"
      + "        { \"name\": \"c\", \"type\":" + innerSchema + ","
      + "          \"default\":{\"c11\": 1, \"c12\": \"hello\"}" + "        }" + "     ]" + "}";

  GenericData.Record writtenRecord;
  GenericData.Record evolvedRecord;
  GenericData.Record innerRecord;

  private static final Schema writer = new Schema.Parser().parse(writerSchema);
  private static final Schema evolved = new Schema.Parser().parse(evolvedSchema2);
  private static final Schema inner = new Schema.Parser().parse(innerSchema);

  @Before
  public void setUp() {
    writtenRecord = new GenericData.Record(writer);
    writtenRecord.put("a", "record");
    writtenRecord.put("b", 21);

    innerRecord = new GenericData.Record(inner);
    innerRecord.put("c11", 1);
    innerRecord.put("c12", "hello");

    evolvedRecord = new GenericData.Record(evolved);
    evolvedRecord.put("a", "record");
    evolvedRecord.put("b", 21);
    evolvedRecord.put("c", innerRecord);
  }

  @Test
  public void testTrevniEvolvedRead() throws IOException {
    AIngleColumnWriter<GenericRecord> acw = new AIngleColumnWriter<>(writer, new ColumnFileMetaData());
    acw.write(writtenRecord);
    File serializedTrevni = File.createTempFile("trevni", null);
    acw.writeTo(serializedTrevni);

    AIngleColumnReader.Params params = new Params(serializedTrevni);
    params.setSchema(evolved);
    try (AIngleColumnReader<GenericRecord> acr = new AIngleColumnReader<>(params)) {
      GenericRecord readRecord = acr.next();
      Assert.assertEquals(evolvedRecord, readRecord);
      Assert.assertFalse(acr.hasNext());
    }
  }

  @Test
  public void testAIngleEvolvedRead() throws IOException {
    File serializedAIngle = File.createTempFile("aingle", null);
    DatumWriter<GenericRecord> dw = new GenericDatumWriter<>(writer);
    DataFileWriter<GenericRecord> dfw = new DataFileWriter<>(dw);
    dfw.create(writer, serializedAIngle);
    dfw.append(writtenRecord);
    dfw.flush();
    dfw.close();

    GenericDatumReader<GenericRecord> reader = new GenericDatumReader<>(writer);
    reader.setExpected(evolved);
    try (DataFileReader<GenericRecord> dfr = new DataFileReader<>(serializedAIngle, reader)) {
      GenericRecord readRecord = dfr.next();
      Assert.assertEquals(evolvedRecord, readRecord);
      Assert.assertFalse(dfr.hasNext());
    }
  }

}
