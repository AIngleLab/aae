/*

 */
package org.apache.aingle.mojo;

import org.codehaus.plexus.util.FileUtils;
import org.junit.Test;

import java.io.File;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

/**
 * Test the Protocol Mojo.
 */
public class TestProtocolMojo extends AbstractAIngleMojoTest {

  private File testPom = new File(getBasedir(), "src/test/resources/unit/protocol/pom.xml");
  private File injectingVelocityToolsTestPom = new File(getBasedir(),
      "src/test/resources/unit/protocol/pom-injecting-velocity-tools.xml");

  @Test
  public void testProtocolMojo() throws Exception {
    final ProtocolMojo mojo = (ProtocolMojo) lookupMojo("protocol", testPom);

    assertNotNull(mojo);
    mojo.execute();

    final File outputDir = new File(getBasedir(), "target/test-harness/protocol/test");
    final Set<String> generatedFiles = new HashSet<>(
        Arrays.asList("ProtocolPrivacy.java", "ProtocolTest.java", "ProtocolUser.java"));

    assertFilesExist(outputDir, generatedFiles);

    final String protocolUserContent = FileUtils.fileRead(new File(outputDir, "ProtocolUser.java"));
    assertTrue("Got " + protocolUserContent + " instead", protocolUserContent.contains("java.time.Instant"));
  }

  @Test
  public void testSetCompilerVelocityAdditionalTools() throws Exception {
    ProtocolMojo mojo = (ProtocolMojo) lookupMojo("protocol", injectingVelocityToolsTestPom);

    assertNotNull(mojo);
    mojo.execute();

    File outputDir = new File(getBasedir(), "target/test-harness/protocol-inject/test");
    final Set<String> generatedFiles = new HashSet<>(
        Arrays.asList("ProtocolPrivacy.java", "ProtocolTest.java", "ProtocolUser.java"));

    assertFilesExist(outputDir, generatedFiles);

    String schemaUserContent = FileUtils.fileRead(new File(outputDir, "ProtocolUser.java"));
    assertTrue(schemaUserContent.contains("It works!"));
  }
}
