/*

 */
package org.apache.aingle.mojo;

import java.io.File;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import org.apache.maven.plugin.testing.AbstractMojoTestCase;

/**
 * Base class for all AIngle mojo test classes.
 */
public abstract class AbstractAIngleMojoTest extends AbstractMojoTestCase {

  @Override
  protected void setUp() throws Exception {
    super.setUp();
  }

  @Override
  protected void tearDown() throws Exception {
    super.tearDown();
  }

  /**
   * Assert the existence files in the given given directory.
   *
   * @param directory     the directory being checked
   * @param expectedFiles the files whose existence is being checked.
   */
  void assertFilesExist(File directory, Set<String> expectedFiles) {
    assertNotNull(directory);
    assertTrue("Directory " + directory.toString() + " does not exists", directory.exists());
    assertNotNull(expectedFiles);
    assertTrue(expectedFiles.size() > 0);

    final Set<String> filesInDirectory = new HashSet<>(Arrays.asList(directory.list()));

    assertEquals(expectedFiles, filesInDirectory);

  }
}
