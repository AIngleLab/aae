/*

 */

package org.apache.aingle.mojo;

import org.apache.aingle.generic.GenericData.StringType;

import java.io.File;
import java.io.IOException;
import java.net.URLClassLoader;

import org.apache.aingle.Protocol;
import org.apache.aingle.compiler.specific.SpecificCompiler;
import org.apache.maven.artifact.DependencyResolutionRequiredException;

/**
 * Generate Java classes and interfaces from AIngle protocol files (.avpr)
 *
 * @goal protocol
 * @phase generate-sources
 * @requiresDependencyResolution runtime
 * @threadSafe
 */
public class ProtocolMojo extends AbstractAIngleMojo {
  /**
   * A set of Ant-like inclusion patterns used to select files from the source
   * directory for processing. By default, the pattern <code>**&#47;*.avpr</code>
   * is used to select grammar files.
   *
   * @parameter
   */
  private String[] includes = new String[] { "**/*.avpr" };

  /**
   * A set of Ant-like inclusion patterns used to select files from the source
   * directory for processing. By default, the pattern <code>**&#47;*.avpr</code>
   * is used to select grammar files.
   *
   * @parameter
   */
  private String[] testIncludes = new String[] { "**/*.avpr" };

  @Override
  protected void doCompile(String filename, File sourceDirectory, File outputDirectory) throws IOException {
    final File src = new File(sourceDirectory, filename);
    final Protocol protocol = Protocol.parse(src);
    final SpecificCompiler compiler = new SpecificCompiler(protocol);
    compiler.setTemplateDir(templateDirectory);
    compiler.setStringType(StringType.valueOf(stringType));
    compiler.setFieldVisibility(getFieldVisibility());
    compiler.setCreateOptionalGetters(createOptionalGetters);
    compiler.setGettersReturnOptional(gettersReturnOptional);
    compiler.setOptionalGettersForNullableFieldsOnly(optionalGettersForNullableFieldsOnly);
    compiler.setCreateSetters(createSetters);
    compiler.setAdditionalVelocityTools(instantiateAdditionalVelocityTools());
    compiler.setEnableDecimalLogicalType(enableDecimalLogicalType);
    final URLClassLoader classLoader;
    try {
      classLoader = createClassLoader();
      for (String customConversion : customConversions) {
        compiler.addCustomConversion(classLoader.loadClass(customConversion));
      }
    } catch (DependencyResolutionRequiredException | ClassNotFoundException e) {
      throw new IOException(e);
    }
    compiler.setOutputCharacterEncoding(project.getProperties().getProperty("project.build.sourceEncoding"));
    compiler.compileToDestination(src, outputDirectory);
  }

  @Override
  protected String[] getIncludes() {
    return includes;
  }

  @Override
  protected String[] getTestIncludes() {
    return testIncludes;
  }
}
