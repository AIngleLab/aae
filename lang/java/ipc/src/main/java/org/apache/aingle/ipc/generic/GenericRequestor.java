/*

 */

package org.apache.aingle.ipc.generic;

import java.io.IOException;

import org.apache.aingle.AIngleRemoteException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericDatumReader;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.ipc.Requestor;
import org.apache.aingle.ipc.Transceiver;

/** {@link Requestor} implementation for generic Java data. */
public class GenericRequestor extends Requestor {
  GenericData data;

  public GenericRequestor(Protocol protocol, Transceiver transceiver) throws IOException {
    this(protocol, transceiver, GenericData.get());
  }

  public GenericRequestor(Protocol protocol, Transceiver transceiver, GenericData data) throws IOException {
    super(protocol, transceiver);
    this.data = data;
  }

  public GenericData getGenericData() {
    return data;
  }

  @Override
  public void writeRequest(Schema schema, Object request, Encoder out) throws IOException {
    new GenericDatumWriter<>(schema, data).write(request, out);
  }

  @Override
  public Object readResponse(Schema writer, Schema reader, Decoder in) throws IOException {
    return new GenericDatumReader<>(writer, reader, data).read(null, in);
  }

  @Override
  public Exception readError(Schema writer, Schema reader, Decoder in) throws IOException {
    Object error = new GenericDatumReader<>(writer, reader, data).read(null, in);
    if (error instanceof CharSequence)
      return new AIngleRuntimeException(error.toString()); // system error
    return new AIngleRemoteException(error);
  }

}
