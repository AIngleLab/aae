/*

 */

package org.apache.aingle.mojo;

import org.apache.aingle.generic.GenericData.StringType;

import java.io.File;
import java.io.IOException;
import java.net.URLClassLoader;

import org.apache.aingle.Schema;
import org.apache.aingle.compiler.specific.SpecificCompiler;
import org.apache.maven.artifact.DependencyResolutionRequiredException;

/**
 * Generate Java classes from AIngle schema files (.ain)
 *
 * @goal schema
 * @phase generate-sources
 * @requiresDependencyResolution runtime+test
 * @threadSafe
 */
public class SchemaMojo extends AbstractAIngleMojo {
  /**
   * A parser used to parse all schema files. Using a common parser will
   * facilitate the import of external schemas.
   */
  private Schema.Parser schemaParser = new Schema.Parser();

  /**
   * A set of Ant-like inclusion patterns used to select files from the source
   * directory for processing. By default, the pattern <code>**&#47;*.ain</code>
   * is used to select grammar files.
   *
   * @parameter
   */
  private String[] includes = new String[] { "**/*.ain" };

  /**
   * A set of Ant-like inclusion patterns used to select files from the source
   * directory for processing. By default, the pattern <code>**&#47;*.ain</code>
   * is used to select grammar files.
   *
   * @parameter
   */
  private String[] testIncludes = new String[] { "**/*.ain" };

  @Override
  protected void doCompile(String filename, File sourceDirectory, File outputDirectory) throws IOException {
    File src = new File(sourceDirectory, filename);
    final Schema schema;

    // This is necessary to maintain backward-compatibility. If there are
    // no imported files then isolate the schemas from each other, otherwise
    // allow them to share a single schema so reuse and sharing of schema
    // is possible.
    if (imports == null) {
      schema = new Schema.Parser().parse(src);
    } else {
      schema = schemaParser.parse(src);
    }

    final SpecificCompiler compiler = new SpecificCompiler(schema);
    compiler.setTemplateDir(templateDirectory);
    compiler.setStringType(StringType.valueOf(stringType));
    compiler.setFieldVisibility(getFieldVisibility());
    compiler.setCreateOptionalGetters(createOptionalGetters);
    compiler.setGettersReturnOptional(gettersReturnOptional);
    compiler.setOptionalGettersForNullableFieldsOnly(optionalGettersForNullableFieldsOnly);
    compiler.setCreateSetters(createSetters);
    compiler.setEnableDecimalLogicalType(enableDecimalLogicalType);
    try {
      final URLClassLoader classLoader = createClassLoader();
      for (String customConversion : customConversions) {
        compiler.addCustomConversion(classLoader.loadClass(customConversion));
      }
    } catch (ClassNotFoundException | DependencyResolutionRequiredException e) {
      throw new IOException(e);
    }
    compiler.setOutputCharacterEncoding(project.getProperties().getProperty("project.build.sourceEncoding"));
    compiler.setAdditionalVelocityTools(instantiateAdditionalVelocityTools());
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
