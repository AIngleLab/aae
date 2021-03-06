/*

 */
package org.apache.aingle;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.IOException;

import org.junit.Test;

import org.apache.aingle.Protocol.Message;

public class TestProtocolParsing {
  public static Protocol getSimpleProtocol() throws IOException {
    File file = new File("../../../share/test/schemas/simple.avpr");
    return Protocol.parse(file);
  }

  @Test
  public void testParsing() throws IOException {
    Protocol protocol = getSimpleProtocol();

    assertEquals(protocol.getDoc(), "Protocol used for testing.");
    assertEquals(6, protocol.getMessages().size());
    assertEquals("Pretend you're in a cave!", protocol.getMessages().get("echo").getDoc());
  }

  private static Message parseMessage(String message) throws Exception {
    return Protocol.parse("{\"protocol\": \"org.foo.Bar\"," + "\"types\": []," + "\"messages\": {" + message + "}}")
        .getMessages().values().iterator().next();
  }

  @Test
  public void oneWay() throws Exception {
    Message m;
    // permit one-way messages w/ null response
    m = parseMessage("\"ack\": {" + "\"request\": []," + "\"response\": \"null\"," + "\"one-way\": true}");
    assertTrue(m.isOneWay());
    // permit one-way messages w/o response
    m = parseMessage("\"ack\": {" + "\"request\": []," + "\"one-way\": true}");
    assertTrue(m.isOneWay());
  }

  @Test(expected = SchemaParseException.class)
  public void oneWayResponse() throws Exception {
    // prohibit one-way messages with a non-null response type
    parseMessage("\"ack\": {" + "\"request\": [\"string\"]," + "\"response\": \"string\"," + "\"one-way\": true}");
  }

  @Test(expected = SchemaParseException.class)
  public void oneWayError() throws Exception {
    // prohibit one-way messages with errors
    parseMessage("\"ack\": {" + "\"request\": [\"string\"]," + "\"errors\": []," + "\"one-way\": true}");
  }

  @Test
  public void testMessageFieldAliases() throws IOException {
    Protocol protocol = getSimpleProtocol();
    final Message msg = protocol.getMessages().get("hello");
    assertNotNull(msg);
    final Schema.Field field = msg.getRequest().getField("greeting");
    assertNotNull(field);
    assertTrue(field.aliases().contains("salute"));
  }

  @Test
  public void testMessageCustomProperties() throws IOException {
    Protocol protocol = getSimpleProtocol();
    final Message msg = protocol.getMessages().get("hello");
    assertNotNull(msg);
    final Schema.Field field = msg.getRequest().getField("greeting");
    assertNotNull(field);
    assertEquals("customValue", field.getProp("customProp"));
  }
}
