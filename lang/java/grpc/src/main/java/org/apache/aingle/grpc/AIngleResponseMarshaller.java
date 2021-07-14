/*

 */

package org.apache.aingle.grpc;

import com.google.common.io.ByteStreams;
import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.Protocol;
import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.specific.SpecificDatumWriter;
import org.apache.aingle.util.Utf8;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import io.grpc.MethodDescriptor;
import io.grpc.Status;

/** Marshaller for AIngle RPC response. */
public class AIngleResponseMarshaller implements MethodDescriptor.Marshaller<Object> {
  private static final EncoderFactory ENCODER_FACTORY = new EncoderFactory();
  private static final DecoderFactory DECODER_FACTORY = new DecoderFactory();
  private final Protocol.Message message;

  public AIngleResponseMarshaller(Protocol.Message message) {
    this.message = message;
  }

  @Override
  public InputStream stream(Object value) {
    return new AIngleResponseInputStream(value, message);
  }

  @Override
  public Object parse(InputStream stream) {
    try {
      if (message.isOneWay())
        return null;
      BinaryDecoder in = DECODER_FACTORY.binaryDecoder(stream, null);
      if (!in.readBoolean()) {
        Object response = new SpecificDatumReader(message.getResponse()).read(null, in);
        return response;
      } else {
        Object value = new SpecificDatumReader(message.getErrors()).read(null, in);
        if (value instanceof Exception) {
          return value;
        }
        return new AIngleRuntimeException(value.toString());
      }
    } catch (IOException e) {
      throw Status.INTERNAL.withCause(e).withDescription("Error deserializing aingle response").asRuntimeException();
    } finally {
      AIngleGrpcUtils.skipAndCloseQuietly(stream);
    }
  }

  private static class AIngleResponseInputStream extends AIngleInputStream {
    private final Protocol.Message message;
    private Object response;

    AIngleResponseInputStream(Object response, Protocol.Message message) {
      this.response = response;
      this.message = message;
    }

    @Override
    public int drainTo(OutputStream target) throws IOException {
      int written;
      if (getPartial() != null) {
        written = (int) ByteStreams.copy(getPartial(), target);
      } else {
        written = writeResponse(target);
      }
      return written;
    }

    private int writeResponse(OutputStream target) throws IOException {
      int written;
      if (message.isOneWay()) {
        written = 0;
      } else if (response instanceof Exception) {
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        BinaryEncoder out = ENCODER_FACTORY.binaryEncoder(bao, null);
        try {
          out.writeBoolean(true);
          new SpecificDatumWriter(message.getErrors()).write(response, out);
        } catch (Exception e) {
          bao = new ByteArrayOutputStream();
          out = ENCODER_FACTORY.binaryEncoder(bao, null);
          out.writeBoolean(true);
          new SpecificDatumWriter(Protocol.SYSTEM_ERRORS).write(new Utf8(e.toString()), out);
        }
        out.flush();
        byte[] serializedError = bao.toByteArray();
        target.write(serializedError);
        written = serializedError.length;
      } else {
        CountingOutputStream outputStream = new CountingOutputStream(target);
        BinaryEncoder out = ENCODER_FACTORY.binaryEncoder(outputStream, null);
        out.writeBoolean(false);
        new SpecificDatumWriter(message.getResponse()).write(response, out);
        out.flush();
        written = outputStream.getWrittenCount();
      }
      response = null;
      return written;
    }
  }
}
