/*

 */

package org.apache.aingle.ipc.generic;

import java.io.IOException;

import org.apache.aingle.AIngleRemoteException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.ipc.Responder;

/** {@link Responder} implementation for generic Java data. */
public abstract class GenericResponder extends Responder {
  private GenericData data;

  public GenericResponder(Protocol local) {
    this(local, GenericData.get());

  }

  public GenericResponder(Protocol local, GenericData data) {
    super(local);
    this.data = data;
  }

  public GenericData getGenericData() {
    return data;
  }

  protected DatumWriter<Object> getDatumWriter(Schema schema) {
    return new GenericDatumWriter<>(schema, data);
  }

  protected DatumReader<Object> getDatumReader(Schema actual, Schema expected) {
    return new GenericDatumReader<>(actual, expected, data);
  }

  @Override
  public Object readRequest(Schema actual, Schema expected, Decoder in) throws IOException {
    return getDatumReader(actual, expected).read(null, in);
  }

  @Override
  public void writeResponse(Schema schema, Object response, Encoder out) throws IOException {
    getDatumWriter(schema).write(response, out);
  }

  @Override
  public void writeError(Schema schema, Object error, Encoder out) throws IOException {
    if (error instanceof AIngleRemoteException)
      error = ((AIngleRemoteException) error).getValue();
    getDatumWriter(schema).write(error, out);
  }

}
