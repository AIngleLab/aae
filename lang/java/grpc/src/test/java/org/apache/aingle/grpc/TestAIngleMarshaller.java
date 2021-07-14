/**

 */

package org.apache.aingle.grpc;

import org.apache.aingle.Protocol;
import org.apache.aingle.grpc.test.Kind;
import org.apache.aingle.grpc.test.MD5;
import org.apache.aingle.grpc.test.TestRecord;
import org.apache.aingle.grpc.test.TestService;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.Random;

import io.grpc.Drainable;

import static org.junit.Assert.assertEquals;

public class TestAIngleMarshaller {
  private final TestRecord record = TestRecord.newBuilder().setName("foo").setKind(Kind.FOO)
      .setArrayOfLongs(Arrays.asList(42L, 424L, 4242L)).setHash(new MD5(new byte[] { 4, 2, 4, 2 }))
      .setNullableHash(null).build();
  private final Protocol.Message message = TestService.PROTOCOL.getMessages().get("echo");
  private Random random = new Random();

  private void readPratialAndDrain(int partialToRead, InputStream inputStream, OutputStream target) throws IOException {
    // read specified partial bytes from request InputStream to target and then
    // drain the rest.
    for (int i = 0; i < partialToRead; i++) {
      int readByte = inputStream.read();
      if (readByte >= 0) {
        target.write(readByte);
      } else {
        break;
      }
    }
    Drainable drainableRequest = (Drainable) inputStream;
    drainableRequest.drainTo(target);
  }

  @Test
  public void testAIngleRequestReadPartialAndDrain() throws IOException {
    AIngleRequestMarshaller requestMarshaller = new AIngleRequestMarshaller(message);
    InputStream requestInputStream = requestMarshaller.stream(new Object[] { record });
    ByteArrayOutputStream requestOutputStream = new ByteArrayOutputStream();
    readPratialAndDrain(random.nextInt(7) + 1, requestInputStream, requestOutputStream);
    InputStream serialized = new ByteArrayInputStream(requestOutputStream.toByteArray());
    Object[] parsedArgs = requestMarshaller.parse(serialized);
    assertEquals(1, parsedArgs.length);
    assertEquals(record, parsedArgs[0]);
  }

  @Test
  public void testAIngleResponseReadPartialAndDrain() throws IOException {
    AIngleResponseMarshaller responseMarshaller = new AIngleResponseMarshaller(message);
    InputStream responseInputStream = responseMarshaller.stream(record);
    ByteArrayOutputStream responseOutputStream = new ByteArrayOutputStream();
    readPratialAndDrain(random.nextInt(7) + 1, responseInputStream, responseOutputStream);
    InputStream serialized = new ByteArrayInputStream(responseOutputStream.toByteArray());
    Object parsedResponse = responseMarshaller.parse(serialized);
    assertEquals(record, parsedResponse);
  }
}
