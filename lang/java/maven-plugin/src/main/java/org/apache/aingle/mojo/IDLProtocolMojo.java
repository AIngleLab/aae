/*

 */

package org.apache.aingle.mojo;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.List;

import org.apache.aingle.Protocol;
import org.apache.aingle.compiler.idl.Idl;
import org.apache.aingle.compiler.idl.ParseException;
import org.apache.aingle.compiler.specific.SpecificCompiler;
import org.apache.aingle.generic.GenericData;

import org.apache.maven.artifact.DependencyResolutionRequiredException;

/**
 * Generate Java classes and interfaces from AIngleIDL files (.avdl)
 *
 * @goal idl-protocol
 * @requiresDependencyResolution runtime
 * @phase generate-sources
 * @threadSafe
 */
public class IDLProtocolMojo extends AbstractAIngleMojo {
  /**
   * A set of Ant-like inclusion patterns used to select files from the source
   * directory for processing. By default, the pattern <code>**&#47;*.avdl</code>
   * is used to select IDL files.
   *
   * @parameter
   */
  private String[] includes = new String[] { "**/*.avdl" };

  /**
   * A set of Ant-like inclusion patterns used to select files from the source
   * directory for processing. By default, the pattern <code>**&#47;*.avdl</code>
   * is used to select IDL files.
   *
   * @parameter
   */
  private String[] testIncludes = new String[] { "**/*.avdl" };

  @Override
  protected void doCompile(String filename, File sourceDirectory, File outputDirectory) throws IOException {
    try {
      @SuppressWarnings("rawtypes")
      List runtimeClasspathElements = project.getRuntimeClasspathElements();

      List<URL> runtimeUrls = new ArrayList<>();

      // Add the source directory of aingle files to the classpath so that
      // imports can refer to other idl files as classpath resources
      runtimeUrls.add(sourceDirectory.toURI().toURL());

      // If runtimeClasspathElements is not empty values add its values to Idl path.
      if (runtimeClasspathElements != null && !runtimeClasspathElements.isEmpty()) {
        for (Object runtimeClasspathElement : runtimeClasspathElements) {
          String element = (String) runtimeClasspathElement;
          runtimeUrls.add(new File(element).toURI().toURL());
        }
      }

      URLClassLoader projPathLoader = new URLClassLoader(runtimeUrls.toArray(new URL[0]),
          Thread.currentThread().getContextClassLoader());
      try (Idl parser = new Idl(new File(sourceDirectory, filename), projPathLoader)) {

        Protocol p = parser.CompilationUnit();
        String json = p.toString(true);
        Protocol protocol = Protocol.parse(json);
        final SpecificCompiler compiler = new SpecificCompiler(protocol);
        compiler.setStringType(GenericData.StringType.valueOf(stringType));
        compiler.setTemplateDir(templateDirectory);
        compiler.setFieldVisibility(getFieldVisibility());
        compiler.setCreateOptionalGetters(createOptionalGetters);
        compiler.setGettersReturnOptional(gettersReturnOptional);
        compiler.setOptionalGettersForNullableFieldsOnly(optionalGettersForNullableFieldsOnly);
        compiler.setCreateSetters(createSetters);
        compiler.setAdditionalVelocityTools(instantiateAdditionalVelocityTools());
        compiler.setEnableDecimalLogicalType(enableDecimalLogicalType);
        for (String customConversion : customConversions) {
          compiler.addCustomConversion(projPathLoader.loadClass(customConversion));
        }
        compiler.setOutputCharacterEncoding(project.getProperties().getProperty("project.build.sourceEncoding"));
        compiler.compileToDestination(null, outputDirectory);
      }
    } catch (ParseException | ClassNotFoundException | DependencyResolutionRequiredException e) {
      throw new IOException(e);
    }
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
