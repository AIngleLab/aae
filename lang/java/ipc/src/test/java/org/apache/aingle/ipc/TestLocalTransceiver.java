/*

 */
package org.apache.aingle.ipc;

import static org.junit.Assert.assertEquals;

import org.apache.aingle.AIngleRemoteException;
import org.apache.aingle.Protocol;
import org.apache.aingle.Protocol.Message;
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.generic.GenericRecord;
import org.apache.aingle.ipc.generic.GenericRequestor;
import org.apache.aingle.ipc.generic.GenericResponder;
import org.apache.aingle.util.Utf8;
import org.junit.Test;

public class TestLocalTransceiver {

  Protocol protocol = Protocol.parse("" + "{\"protocol\": \"Minimal\", " + "\"messages\": { \"m\": {"
      + "   \"request\": [{\"name\": \"x\", \"type\": \"string\"}], " + "   \"response\": \"string\"} } }");

  static class TestResponder extends GenericResponder {
    public TestResponder(Protocol local) {
      super(local);
    }

    @Override
    public Object respond(Message message, Object request) throws AIngleRemoteException {
      assertEquals(new Utf8("hello"), ((GenericRecord) request).get("x"));
      return new Utf8("there");
    }

  }

  @Test
  public void testSingleRpc() throws Exception {
    Transceiver t = new LocalTransceiver(new TestResponder(protocol));
    GenericRecord params = new GenericData.Record(protocol.getMessages().get("m").getRequest());
    params.put("x", new Utf8("hello"));
    GenericRequestor r = new GenericRequestor(protocol, t);

    for (int x = 0; x < 5; x++)
      assertEquals(new Utf8("there"), r.request("m", params));
  }

}
