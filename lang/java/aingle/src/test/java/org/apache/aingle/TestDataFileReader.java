/*

 */
package org.apache.aingle;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.lang.management.OperatingSystemMXBean;
import java.nio.file.Files;
import java.nio.file.Path;
import com.sun.management.UnixOperatingSystemMXBean;
import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileStream;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.file.FileReader;
import org.apache.aingle.file.SeekableFileInput;
import org.apache.aingle.file.SeekableInput;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.junit.Test;

@SuppressWarnings("restriction")
public class TestDataFileReader {

  @Test
  // regression test for bug AINGLE-2286
  public void testForLeakingFileDescriptors() throws IOException {
    StringBuilder sb = new StringBuilder();
    int maxTries = 3;
    for (int tries = 0; tries < maxTries; tries++) {
      Path emptyFile = Files.createTempFile("empty", ".aingle");
      Files.deleteIfExists(emptyFile);
      Files.createFile(emptyFile);

      long openFilesBeforeOperation = getNumberOfOpenFileDescriptors();
      try (DataFileReader<Object> reader = new DataFileReader<>(emptyFile.toFile(), new GenericDatumReader<>())) {
        fail("Reading on empty file is supposed to fail.");
      } catch (IOException e) {
        // everything going as supposed to
      }
      Files.delete(emptyFile);

      long openFilesAfterOperation = getNumberOfOpenFileDescriptors();
      if (openFilesBeforeOperation == openFilesAfterOperation)
        return;

      // Sometimes the number of file descriptors is off due to other processes or
      // garbage
      // collection. We note each inconsistency and retry.
      sb.append(openFilesBeforeOperation).append("!=").append(openFilesAfterOperation).append(",");
    }
    fail("File descriptor leaked from new DataFileReader() over " + maxTries + " tries: ("
        + sb.substring(0, sb.length() - 1) + ")");
  }

  private long getNumberOfOpenFileDescriptors() {
    OperatingSystemMXBean osMxBean = ManagementFactory.getOperatingSystemMXBean();
    if (osMxBean instanceof UnixOperatingSystemMXBean) {
      return ((UnixOperatingSystemMXBean) osMxBean).getOpenFileDescriptorCount();
    }
    return 0;
  }

  @Test
  // regression test for bug AINGLE-2944
  public void testThrottledInputStream() throws IOException {
    // AINGLE-2944 describes hanging/failure in reading AIngle file with performing
    // magic header check. This happens with throttled input stream,
    // where we read into buffer less bytes than requested.

    Schema legacySchema = new Schema.Parser().setValidate(false).setValidateDefaults(false)
        .parse("{\"type\": \"record\", \"name\": \"TestSchema\", \"fields\": "
            + "[ {\"name\": \"id\", \"type\": [\"long\", \"null\"], \"default\": null}]}");
    File f = Files.createTempFile("testThrottledInputStream", ".aingle").toFile();
    try (DataFileWriter<?> w = new DataFileWriter<>(new GenericDatumWriter<>())) {
      w.create(legacySchema, f);
      w.flush();
    }

    // Without checking for magic header, throttled input has no effect
    FileReader r = new DataFileReader(throttledInputStream(f), new GenericDatumReader<>());
    assertEquals("TestSchema", r.getSchema().getName());

    // With checking for magic header, throttled input should pass too.
    FileReader r2 = DataFileReader.openReader(throttledInputStream(f), new GenericDatumReader<>());
    assertEquals("TestSchema", r2.getSchema().getName());
  }

  private SeekableInput throttledInputStream(File f) throws IOException {
    SeekableFileInput input = new SeekableFileInput(f);
    return new SeekableInput() {
      @Override
      public void close() throws IOException {
        input.close();
      }

      @Override
      public void seek(long p) throws IOException {
        input.seek(p);
      }

      @Override
      public long tell() throws IOException {
        return input.tell();
      }

      @Override
      public long length() throws IOException {
        return input.length();
      }

      @Override
      public int read(byte[] b, int off, int len) throws IOException {
        if (len == 1) {
          return input.read(b, off, len);
        } else {
          return input.read(b, off, len - 1);
        }
      }
    };
  }

  @Test(expected = EOFException.class)
  // another regression test for bug AINGLE-2944, testing EOF case
  public void testInputStreamEOF() throws IOException {
    // AINGLE-2944 describes hanging/failure in reading AIngle file with performing
    // magic header check. This potentially happens with a defective input stream
    // where a -1 value is unexpectedly returned from a read.
    Schema legacySchema = new Schema.Parser().setValidate(false).setValidateDefaults(false)
        .parse("{\"type\": \"record\", \"name\": \"TestSchema\", \"fields\": "
            + "[ {\"name\": \"id\", \"type\": [\"long\", \"null\"], \"default\": null}]}");
    File f = Files.createTempFile("testInputStreamEOF", ".aingle").toFile();
    try (DataFileWriter<?> w = new DataFileWriter<>(new GenericDatumWriter<>())) {
      w.create(legacySchema, f);
      w.flush();
    }

    // Should throw an EOFException
    DataFileReader.openReader(eofInputStream(f), new GenericDatumReader<>());
  }

  private SeekableInput eofInputStream(File f) throws IOException {
    SeekableFileInput input = new SeekableFileInput(f);
    return new SeekableInput() {
      @Override
      public void close() throws IOException {
        input.close();
      }

      @Override
      public void seek(long p) throws IOException {
        input.seek(p);
      }

      @Override
      public long tell() throws IOException {
        return input.tell();
      }

      @Override
      public long length() throws IOException {
        return input.length();
      }

      @Override
      public int read(byte[] b, int off, int len) throws IOException {
        return -1;
      }
    };
  }

  @Test
  public void testIgnoreSchemaValidationOnRead() throws IOException {
    // This schema has an accent in the name and the default for the field doesn't
    // match the first type in the union. A Java SDK in the past could create a file
    // containing this schema.
    Schema legacySchema = new Schema.Parser().setValidate(false).setValidateDefaults(false)
        .parse("{\"type\": \"record\", \"name\": \"InvalidAccëntWithInvalidNull\", \"fields\": "
            + "[ {\"name\": \"id\", \"type\": [\"long\", \"null\"], \"default\": null}]}");

    // Create a file with the legacy schema.
    File f = Files.createTempFile("testIgnoreSchemaValidationOnRead", ".aingle").toFile();
    try (DataFileWriter<?> w = new DataFileWriter<>(new GenericDatumWriter<>())) {
      w.create(legacySchema, f);
      w.flush();
    }

    // This should not throw an exception.
    try (DataFileStream<Void> r = new DataFileStream<>(new FileInputStream(f), new GenericDatumReader<>())) {
      assertEquals(legacySchema, r.getSchema());
    }
  }

  @Test(expected = InvalidAIngleMagicException.class)
  public void testInvalidMagicLength() throws IOException {
    File f = Files.createTempFile("testInvalidMagicLength", ".aingle").toFile();
    try (FileWriter w = new FileWriter(f)) {
      w.write("-");
    }

    DataFileReader.openReader(new SeekableFileInput(f), new GenericDatumReader<>());
  }

  @Test(expected = InvalidAIngleMagicException.class)
  public void testInvalidMagicBytes() throws IOException {
    File f = Files.createTempFile("testInvalidMagicBytes", ".aingle").toFile();
    try (FileWriter w = new FileWriter(f)) {
      w.write("invalid");
    }

    DataFileReader.openReader(new SeekableFileInput(f), new GenericDatumReader<>());
  }
}
