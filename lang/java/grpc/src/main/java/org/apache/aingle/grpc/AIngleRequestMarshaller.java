/*

 */

package org.apache.aingle.grpc;

import com.google.common.io.ByteStreams;
import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.io.BinaryDecoder;
import org.apache.aingle.io.BinaryEncoder;
import org.apache.aingle.io.DecoderFactory;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.specific.SpecificDatumWriter;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import io.grpc.MethodDescriptor;
import io.grpc.Status;

/** Marshaller for AIngle RPC request. */
public class AIngleRequestMarshaller implements MethodDescriptor.Marshaller<Object[]> {
  private static final EncoderFactory ENCODER_FACTORY = new EncoderFactory();
  private static final DecoderFactory DECODER_FACTORY = new DecoderFactory();
  private final Protocol.Message message;

  public AIngleRequestMarshaller(Protocol.Message message) {
    this.message = message;
  }

  @Override
  public InputStream stream(Object[] value) {
    return new AIngleRequestInputStream(value, message);
  }

  @Override
  public Object[] parse(InputStream stream) {
    try {
      BinaryDecoder in = DECODER_FACTORY.binaryDecoder(stream, null);
      Schema reqSchema = message.getRequest();
      GenericRecord request = (GenericRecord) new SpecificDatumReader<>(reqSchema).read(null, in);
      Object[] args = new Object[reqSchema.getFields().size()];
      int i = 0;
      for (Schema.Field field : reqSchema.getFields()) {
        args[i++] = request.get(field.name());
      }
      return args;
    } catch (IOException e) {
      throw Status.INTERNAL.withCause(e).withDescription("Error deserializing aingle request arguments")
          .asRuntimeException();
    } finally {
      AIngleGrpcUtils.skipAndCloseQuietly(stream);
    }
  }

  private static class AIngleRequestInputStream extends AIngleInputStream {
    private final Protocol.Message message;
    private Object[] args;

    AIngleRequestInputStream(Object[] args, Protocol.Message message) {
      this.args = args;
      this.message = message;
    }

    @Override
    public int drainTo(OutputStream target) throws IOException {
      int written;
      if (getPartial() != null) {
        written = (int) ByteStreams.copy(getPartial(), target);
      } else {
        Schema reqSchema = message.getRequest();
        CountingOutputStream outputStream = new CountingOutputStream(target);
        BinaryEncoder out = ENCODER_FACTORY.binaryEncoder(outputStream, null);
        int i = 0;
        for (Schema.Field param : reqSchema.getFields()) {
          new SpecificDatumWriter<>(param.schema()).write(args[i++], out);
        }
        out.flush();
        args = null;
        written = outputStream.getWrittenCount();
      }
      return written;
    }
  }
}
