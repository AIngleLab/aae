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
 * Test the Schema Mojo.
 */
public class TestSchemaMojo extends AbstractAIngleMojoTest {

  private File testPom = new File(getBasedir(), "src/test/resources/unit/schema/pom.xml");
  private File injectingVelocityToolsTestPom = new File(getBasedir(),
      "src/test/resources/unit/schema/pom-injecting-velocity-tools.xml");

  @Test
  public void testSchemaMojo() throws Exception {
    final SchemaMojo mojo = (SchemaMojo) lookupMojo("schema", testPom);

    assertNotNull(mojo);
    mojo.execute();

    final File outputDir = new File(getBasedir(), "target/test-harness/schema/test");
    final Set<String> generatedFiles = new HashSet<>(
        Arrays.asList("PrivacyDirectImport.java", "PrivacyImport.java", "SchemaPrivacy.java", "SchemaUser.java"));

    assertFilesExist(outputDir, generatedFiles);

    final String schemaUserContent = FileUtils.fileRead(new File(outputDir, "SchemaUser.java"));
    assertTrue(schemaUserContent.contains("java.time.Instant"));
  }

  @Test
  public void testSetCompilerVelocityAdditionalTools() throws Exception {
    final SchemaMojo mojo = (SchemaMojo) lookupMojo("schema", injectingVelocityToolsTestPom);

    assertNotNull(mojo);
    mojo.execute();

    final File outputDir = new File(getBasedir(), "target/test-harness/schema-inject/test");
    final Set<String> generatedFiles = new HashSet<>(
        Arrays.asList("PrivacyDirectImport.java", "PrivacyImport.java", "SchemaPrivacy.java", "SchemaUser.java"));

    assertFilesExist(outputDir, generatedFiles);

    final String schemaUserContent = FileUtils.fileRead(new File(outputDir, "SchemaUser.java"));
    assertTrue("Got " + schemaUserContent + " instead", schemaUserContent.contains("It works!"));
  }
}
