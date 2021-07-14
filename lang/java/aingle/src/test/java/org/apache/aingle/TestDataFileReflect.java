/*

 */
package org.apache.aingle;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.file.SeekableFileInput;
import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.reflect.ReflectData;
import org.apache.aingle.reflect.ReflectDatumReader;
import org.apache.aingle.reflect.ReflectDatumWriter;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestDataFileReflect {

  @Rule
  public TemporaryFolder DIR = new TemporaryFolder();

  /*
   * Test that using multiple schemas in a file works doing a union before writing
   * any records.
   */
  @Test
  public void testMultiReflectWithUnionBeforeWriting() throws IOException {
    File file = new File(DIR.getRoot().getPath(), "testMultiReflectWithUnionBeforeWriting.aingle");
    CheckList<Object> check = new CheckList<>();
    try (FileOutputStream fos = new FileOutputStream(file)) {

      ReflectData reflectData = ReflectData.get();
      List<Schema> schemas = Arrays.asList(reflectData.getSchema(FooRecord.class),
          reflectData.getSchema(BarRecord.class));
      Schema union = Schema.createUnion(schemas);

      try (DataFileWriter<Object> writer = new DataFileWriter<>(new ReflectDatumWriter<>(union))) {
        writer.create(union, fos);

        // test writing to a file
        write(writer, new BarRecord("One beer please"), check);
        write(writer, new FooRecord(10), check);
        write(writer, new BarRecord("Two beers please"), check);
        write(writer, new FooRecord(20), check);
      }
    }
    // new File(DIR.getRoot().getPath(), "test.aingle");
    ReflectDatumReader<Object> din = new ReflectDatumReader<>();
    SeekableFileInput sin = new SeekableFileInput(file);
    try (DataFileReader<Object> reader = new DataFileReader<>(sin, din)) {
      int count = 0;
      for (Object datum : reader) {
        check.assertEquals(datum, count++);
      }
      Assert.assertEquals(count, check.size());
    }
  }

  /*
   * Test that writing a record with a field that is null.
   */
  @Test
  public void testNull() throws IOException {
    File file = new File(DIR.getRoot().getPath(), "testNull.aingle");
    CheckList<BarRecord> check = new CheckList<>();

    try (FileOutputStream fos = new FileOutputStream(file)) {
      ReflectData reflectData = ReflectData.AllowNull.get();
      Schema schema = reflectData.getSchema(BarRecord.class);
      try (DataFileWriter<BarRecord> writer = new DataFileWriter<>(
          new ReflectDatumWriter<>(BarRecord.class, reflectData))) {
        writer.create(schema, fos);
        // test writing to a file
        write(writer, new BarRecord("One beer please"), check);
        // null record here, fails when using the default reflectData instance
        write(writer, new BarRecord(), check);
        write(writer, new BarRecord("Two beers please"), check);
      }
    }

    ReflectDatumReader<BarRecord> din = new ReflectDatumReader<>();
    try (SeekableFileInput sin = new SeekableFileInput(file)) {
      try (DataFileReader<BarRecord> reader = new DataFileReader<>(sin, din)) {
        int count = 0;
        for (BarRecord datum : reader) {
          check.assertEquals(datum, count++);
        }
        Assert.assertEquals(count, check.size());
      }
    }
  }

  @Test
  public void testNew() throws IOException {
    ByteBuffer payload = ByteBuffer.allocateDirect(8 * 1024);
    for (int i = 0; i < 500; i++) {
      payload.putInt(1);
    }
    payload.flip();
    ByteBufferRecord bbr = new ByteBufferRecord();
    bbr.setPayload(payload);
    bbr.setTp(TypeEnum.b);

    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    ReflectDatumWriter<ByteBufferRecord> writer = new ReflectDatumWriter<>(ByteBufferRecord.class);
    BinaryEncoder aingleEncoder = EncoderFactory.get().blockingBinaryEncoder(outputStream, null);
    writer.write(bbr, aingleEncoder);
    aingleEncoder.flush();

    byte[] bytes = outputStream.toByteArray();

    ByteArrayInputStream inputStream = new ByteArrayInputStream(bytes);
    ReflectDatumReader<ByteBufferRecord> datumReader = new ReflectDatumReader<>(ByteBufferRecord.class);
    BinaryDecoder aingleDecoder = DecoderFactory.get().binaryDecoder(inputStream, null);
    ByteBufferRecord deserialized = datumReader.read(null, aingleDecoder);

    Assert.assertEquals(bbr, deserialized);
  }

  /*
   * Test that writing out and reading in a nested class works
   */
  @Test
  public void testNestedClass() throws IOException {
    File file = new File(DIR.getRoot().getPath(), "testNull.aingle");

    CheckList<BazRecord> check = new CheckList<>();
    try (FileOutputStream fos = new FileOutputStream(file)) {
      Schema schema = ReflectData.get().getSchema(BazRecord.class);
      try (DataFileWriter<BazRecord> writer = new DataFileWriter<>(new ReflectDatumWriter<>(schema))) {
        writer.create(schema, fos);

        // test writing to a file
        write(writer, new BazRecord(10), check);
        write(writer, new BazRecord(20), check);
      }
    }

    ReflectDatumReader<BazRecord> din = new ReflectDatumReader<>();
    try (SeekableFileInput sin = new SeekableFileInput(file)) {
      try (DataFileReader<BazRecord> reader = new DataFileReader<>(sin, din)) {
        int count = 0;
        for (BazRecord datum : reader) {
          check.assertEquals(datum, count++);
        }
        Assert.assertEquals(count, check.size());
      }
    }
  }

  private <T> void write(DataFileWriter<T> writer, T o, CheckList<T> l) throws IOException {
    writer.append(l.addAndReturn(o));
  }

  @SuppressWarnings("serial")
  private static class CheckList<T> extends ArrayList<T> {
    T addAndReturn(T check) {
      add(check);
      return check;
    }

    void assertEquals(Object toCheck, int i) {
      Assert.assertNotNull(toCheck);
      Object o = get(i);
      Assert.assertNotNull(o);
      Assert.assertEquals(toCheck, o);
    }
  }

  private static class BazRecord {
    private int nbr;

    @SuppressWarnings("unused")
    public BazRecord() {
    }

    public BazRecord(int nbr) {
      this.nbr = nbr;
    }

    @Override
    public boolean equals(Object that) {
      if (that instanceof BazRecord) {
        return this.nbr == ((BazRecord) that).nbr;
      }
      return false;
    }

    @Override
    public int hashCode() {
      return nbr;
    }

    @Override
    public String toString() {
      return BazRecord.class.getSimpleName() + "{cnt=" + nbr + "}";
    }
  }
}
