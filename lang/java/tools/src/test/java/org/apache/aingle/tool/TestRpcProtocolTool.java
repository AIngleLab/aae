/*

 */
package org.apache.aingle.tool;

import org.apache.aingle.Protocol;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.PrintStream;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.junit.Assert.assertEquals;

/**
 *
 */
@RunWith(Parameterized.class)
public class TestRpcProtocolTool {

  @Parameterized.Parameters(/* name = "{0}" */)
  public static List<Object[]> data() {
    return Arrays.asList(new Object[] { "http" }, new Object[] { "aingle" });
  }

  private RpcReceiveTool receive;
  private Protocol simpleProtocol;

  private String uriScheme;

  public TestRpcProtocolTool(String uriScheme) {
    this.uriScheme = uriScheme;
  }

  @Before
  public void setUp() throws Exception {
    String protocolFile = System.getProperty("share.dir", "../../../share") + "/test/schemas/simple.avpr";

    simpleProtocol = Protocol.parse(new File(protocolFile));

    // start a simple server
    ByteArrayOutputStream baos1 = new ByteArrayOutputStream();
    PrintStream p1 = new PrintStream(baos1);
    receive = new RpcReceiveTool();
    receive.run1(null, p1, System.err,
        Arrays.asList(uriScheme + "://0.0.0.0:0/", protocolFile, "hello", "-data", "\"Hello!\""));
  }

  @After
  public void tearDown() throws Exception {
    if (receive != null)
      receive.server.close(); // force the server to finish
  }

  @Test
  public void testRpcProtocol() throws Exception {

    // run the actual test
    ByteArrayOutputStream baos2 = new ByteArrayOutputStream();
    PrintStream p2 = new PrintStream(baos2, true, "UTF-8");
    RpcProtocolTool testObject = new RpcProtocolTool();

    testObject.run(null, p2, System.err,
        Collections.singletonList(uriScheme + "://127.0.0.1:" + receive.server.getPort() + "/"));

    p2.flush();

    assertEquals("Expected the simple.avpr protocol to be echoed to standout", simpleProtocol,
        Protocol.parse(baos2.toString("UTF-8")));

  }
}
