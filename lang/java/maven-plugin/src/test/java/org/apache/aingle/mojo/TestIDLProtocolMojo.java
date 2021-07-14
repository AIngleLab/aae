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
 * Test the IDL Protocol Mojo.
 */
public class TestIDLProtocolMojo extends AbstractAIngleMojoTest {

  private File testPom = new File(getBasedir(), "src/test/resources/unit/idl/pom.xml");
  private File injectingVelocityToolsTestPom = new File(getBasedir(),
      "src/test/resources/unit/idl/pom-injecting-velocity-tools.xml");

  @Test
  public void testIdlProtocolMojo() throws Exception {
    final IDLProtocolMojo mojo = (IDLProtocolMojo) lookupMojo("idl-protocol", testPom);

    assertNotNull(mojo);
    mojo.execute();

    final File outputDir = new File(getBasedir(), "target/test-harness/idl/test/");
    final Set<String> generatedFiles = new HashSet<>(Arrays.asList("IdlPrivacy.java", "IdlTest.java", "IdlUser.java",
        "IdlUserWrapper.java", "IdlClasspathImportTest.java"));
    assertFilesExist(outputDir, generatedFiles);

    final String idlUserContent = FileUtils.fileRead(new File(outputDir, "IdlUser.java"));
    assertTrue(idlUserContent.contains("java.time.Instant"));
  }

  @Test
  public void testSetCompilerVelocityAdditionalTools() throws Exception {
    final IDLProtocolMojo mojo = (IDLProtocolMojo) lookupMojo("idl-protocol", injectingVelocityToolsTestPom);

    assertNotNull(mojo);
    mojo.execute();

    final File outputDir = new File(getBasedir(), "target/test-harness/idl-inject/test");
    final Set<String> generatedFiles = new HashSet<>(Arrays.asList("IdlPrivacy.java", "IdlTest.java", "IdlUser.java",
        "IdlUserWrapper.java", "IdlClasspathImportTest.java"));

    assertFilesExist(outputDir, generatedFiles);

    final String schemaUserContent = FileUtils.fileRead(new File(outputDir, "IdlUser.java"));
    assertTrue(schemaUserContent.contains("It works!"));
  }
}
