/*

 */

package org.apache.aingle.mapred;

import static org.apache.aingle.mapred.AIngleOutputFormat.EXT;

import java.io.IOException;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

import org.apache.aingle.Schema;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.reflect.ReflectDatumWriter;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.RecordWriter;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.util.Progressable;

/**
 * The equivalent of {@link org.apache.hadoop.mapred.TextOutputFormat} for
 * writing to AIngle Data Files with a <code>"bytes"</code> schema.
 */
public class AIngleTextOutputFormat<K, V> extends FileOutputFormat<K, V> {

  @Override
  public RecordWriter<K, V> getRecordWriter(FileSystem ignore, JobConf job, String name, Progressable prog)
      throws IOException {

    Schema schema = Schema.create(Schema.Type.BYTES);

    final byte[] keyValueSeparator = job.get("mapreduce.output.textoutputformat.separator", "\t")
        .getBytes(StandardCharsets.UTF_8);

    final DataFileWriter<ByteBuffer> writer = new DataFileWriter<>(new ReflectDatumWriter<>());

    AIngleOutputFormat.configureDataFileWriter(writer, job);

    Path path = FileOutputFormat.getTaskOutputPath(job, name + EXT);
    writer.create(schema, path.getFileSystem(job).create(path));

    return new AIngleTextRecordWriter(writer, keyValueSeparator);
  }

  class AIngleTextRecordWriter implements RecordWriter<K, V> {
    private final DataFileWriter<ByteBuffer> writer;
    private final byte[] keyValueSeparator;

    public AIngleTextRecordWriter(DataFileWriter<ByteBuffer> writer, byte[] keyValueSeparator) {
      this.writer = writer;
      this.keyValueSeparator = keyValueSeparator;
    }

    @Override
    public void write(K key, V value) throws IOException {
      boolean nullKey = key == null || key instanceof NullWritable;
      boolean nullValue = value == null || value instanceof NullWritable;
      if (nullKey && nullValue) {
        // NO-OP
      } else if (!nullKey && nullValue) {
        writer.append(toByteBuffer(key));
      } else if (nullKey && !nullValue) {
        writer.append(toByteBuffer(value));
      } else {
        writer.append(toByteBuffer(key, keyValueSeparator, value));
      }
    }

    @Override
    public void close(Reporter reporter) throws IOException {
      writer.close();
    }

    private ByteBuffer toByteBuffer(Object o) throws IOException {
      if (o instanceof Text) {
        Text to = (Text) o;
        return ByteBuffer.wrap(to.getBytes(), 0, to.getLength());
      } else {
        return ByteBuffer.wrap(o.toString().getBytes(StandardCharsets.UTF_8));
      }
    }

    private ByteBuffer toByteBuffer(Object key, byte[] sep, Object value) throws IOException {
      byte[] keyBytes, valBytes;
      int keyLength, valLength;
      if (key instanceof Text) {
        Text tkey = (Text) key;
        keyBytes = tkey.getBytes();
        keyLength = tkey.getLength();
      } else {
        keyBytes = key.toString().getBytes(StandardCharsets.UTF_8);
        keyLength = keyBytes.length;
      }
      if (value instanceof Text) {
        Text tval = (Text) value;
        valBytes = tval.getBytes();
        valLength = tval.getLength();
      } else {
        valBytes = value.toString().getBytes(StandardCharsets.UTF_8);
        valLength = valBytes.length;
      }
      ByteBuffer buf = ByteBuffer.allocate(keyLength + sep.length + valLength);
      buf.put(keyBytes, 0, keyLength);
      buf.put(sep);
      buf.put(valBytes, 0, valLength);
      ((Buffer) buf).rewind();
      return buf;
    }

  }

}
