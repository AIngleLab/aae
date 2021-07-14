/*

 */
package org.apache.aingle;

import static org.junit.Assert.*;

import org.junit.Test;

public class TestProtocol {

  @Test
  public void testPropEquals() {
    Protocol p1 = new Protocol("P", null, "foo");
    p1.addProp("a", "1");
    Protocol p2 = new Protocol("P", null, "foo");
    p2.addProp("a", "2");
    assertFalse(p1.equals(p2));
  }

  @Test
  public void testSplitProtocolBuild() {
    Protocol p = new Protocol("P", null, "foo");
    p.addProp("property", "some value");

    String protocolString = p.toString();
    final int mid = protocolString.length() / 2;

    Protocol parsedStringProtocol = org.apache.aingle.Protocol.parse(protocolString);
    Protocol parsedArrayOfStringProtocol = org.apache.aingle.Protocol.parse(protocolString.substring(0, mid),
        protocolString.substring(mid));

    assertNotNull(parsedStringProtocol);
    assertNotNull(parsedArrayOfStringProtocol);
    assertEquals(parsedStringProtocol.toString(), parsedArrayOfStringProtocol.toString());
  }
}
