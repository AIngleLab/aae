/*

 */

package org.apache.aingle.mojo;

import java.io.File;
import java.util.Arrays;

import org.apache.aingle.Protocol;
import org.apache.aingle.Schema;
import org.apache.aingle.entities.Person;
import org.apache.aingle.protocols.Remote;
import org.apache.aingle.reflect.ReflectData;
import org.apache.maven.plugin.testing.AbstractMojoTestCase;
import org.junit.Test;

public class TestInduceMojo extends AbstractMojoTestCase {

  protected File schemaPom;
  protected File protocolPom;

  @Override
  protected void setUp() throws Exception {
    String baseDir = getBasedir();
    schemaPom = new File(baseDir, "src/test/resources/unit/schema/induce-pom.xml");
    protocolPom = new File(baseDir, "src/test/resources/unit/protocol/induce-pom.xml");
    super.setUp();
  }

  @Override
  protected void tearDown() throws Exception {
    super.tearDown();
  }

  public void testInduceMojoExists() throws Exception {
    InduceMojo mojo = (InduceMojo) lookupMojo("induce", schemaPom);

    assertNotNull(mojo);
  }

  @Test
  public void testInduceSchema() throws Exception {
    executeMojo(schemaPom);

    File outputDir = new File(getBasedir(), "target/test-harness/schemas/org/apache/aingle/entities");
    assertTrue(outputDir.listFiles().length != 0);
    File personSchemaFile = Arrays.stream(outputDir.listFiles()).filter(file -> file.getName().endsWith("Person.ain"))
        .findFirst().orElseThrow(AssertionError::new);
    assertEquals(ReflectData.get().getSchema(Person.class), new Schema.Parser().parse(personSchemaFile));
  }

  @Test
  public void testInducedSchemasFileExtension() throws Exception {
    executeMojo(schemaPom);

    File outputDir = new File(getBasedir(), "target/test-harness/schemas/org/apache/aingle/entities");
    for (File file : outputDir.listFiles()) {
      assertTrue(file.getName().contains(".ain"));
    }
  }

  @Test
  public void testInduceProtocol() throws Exception {
    executeMojo(protocolPom);

    File outputDir = new File(getBasedir(), "target/test-harness/protocol/org/apache/aingle/protocols");
    assertTrue(outputDir.listFiles().length != 0);
    File remoteProtocolFile = Arrays.stream(outputDir.listFiles())
        .filter(file -> file.getName().endsWith("Remote.avpr")).findFirst().orElseThrow(AssertionError::new);
    assertEquals(ReflectData.get().getProtocol(Remote.class), Protocol.parse(remoteProtocolFile));
  }

  @Test
  public void testInducedProtocolsFileExtension() throws Exception {
    executeMojo(protocolPom);

    File outputDir = new File(getBasedir(), "target/test-harness/protocol/org/apache/aingle/protocols");
    for (File file : outputDir.listFiles()) {
      assertTrue(file.getName().contains(".avpr"));
    }
  }

  private void executeMojo(File pom) throws Exception {
    InduceMojo mojo = (InduceMojo) lookupMojo("induce", pom);
    mojo.execute();
  }
}
