/*

 */
package org.apache.aingle.reflect;

import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;

/**
 * Expert: a custom encoder and decoder that writes an object directly to aingle.
 * No validation is performed to check that the encoding conforms to the schema.
 * Invalid implementations may result in an unreadable file. The use of
 * {@link org.apache.aingle.io.ValidatingEncoder} is recommended.
 *
 * @param <T> The class of objects that can be serialized with this encoder /
 *            decoder.
 */
public abstract class CustomEncoding<T> {

  protected Schema schema;

  protected abstract void write(Object datum, Encoder out) throws IOException;

  protected abstract T read(Object reuse, Decoder in) throws IOException;

  T read(Decoder in) throws IOException {
    return this.read(null, in);
  }

  protected Schema getSchema() {
    return schema;
  }

}
