/*

 */
package org.apache.aingle.reflect;

import java.io.IOException;
import java.util.Date;

import org.apache.aingle.Schema;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;

/**
 * This encoder/decoder writes a java.util.Date object as a long to aingle and
 * reads a Date object from long. The long stores the number of milliseconds
 * since January 1, 1970, 00:00:00 GMT represented by the Date object.
 */
public class DateAsLongEncoding extends CustomEncoding<Date> {
  {
    schema = Schema.create(Schema.Type.LONG);
    schema.addProp("CustomEncoding", "DateAsLongEncoding");
  }

  @Override
  protected final void write(Object datum, Encoder out) throws IOException {
    out.writeLong(((Date) datum).getTime());
  }

  @Override
  protected final Date read(Object reuse, Decoder in) throws IOException {
    if (reuse instanceof Date) {
      ((Date) reuse).setTime(in.readLong());
      return (Date) reuse;
    } else
      return new Date(in.readLong());
  }

}
