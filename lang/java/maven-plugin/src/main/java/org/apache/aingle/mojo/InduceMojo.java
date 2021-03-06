/*

 */

package org.apache.aingle.mojo;

import java.io.File;
import java.io.PrintWriter;
import java.lang.reflect.Constructor;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.charset.Charset;
import java.util.List;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.reflect.ReflectData;
import org.apache.maven.plugin.AbstractMojo;
import org.apache.maven.plugin.MojoExecutionException;
import org.apache.maven.project.MavenProject;

/**
 * Generate AIngle files (.ain and .avpr) from Java classes or interfaces
 *
 * @goal induce
 * @phase process-classes
 * @threadSafe
 */
public class InduceMojo extends AbstractMojo {
  /**
   * The Java source directories.
   *
   * @parameter property="javaSourceDirectories"
   *            default-value="${basedir}/src/main/java"
   */
  private File[] javaSourceDirectories;

  /**
   * Directory where to output AIngle schemas (.ain) or protocols (.avpr).
   *
   * @parameter property="aingleOutputDirectory"
   *            default-value="${project.build.directory}/generated-resources/aingle"
   */
  private File aingleOutputDirectory;

  /**
   * The output encoding.
   *
   * @parameter default-value="${project.build.sourceEncoding}"
   */
  private String encoding;

  /**
   * Whether to use ReflectData.AllowNull.
   *
   * @parameter default-value="false"
   */
  private boolean allowNull;

  /**
   * Override the default ReflectData implementation with an extension. Must be a
   * subclass of ReflectData.
   *
   * @parameter property="reflectDataImplementation"
   */
  private String reflectDataImplementation;

  /**
   * The current Maven project.
   *
   * @parameter default-value="${project}"
   * @readonly
   * @required
   */
  protected MavenProject project;

  private ClassLoader classLoader;
  private ReflectData reflectData;

  public void execute() throws MojoExecutionException {
    classLoader = getClassLoader();
    reflectData = getReflectData();

    if (encoding == null) {
      encoding = Charset.defaultCharset().name();
      getLog().warn("Property project.build.sourceEncoding not set, using system default " + encoding);
    }

    for (File sourceDirectory : javaSourceDirectories) {
      induceClasses(sourceDirectory);
    }
  }

  private void induceClasses(File sourceDirectory) throws MojoExecutionException {
    File[] files = sourceDirectory.listFiles();
    if (files == null) {
      throw new MojoExecutionException("Unable to list files from directory: " + sourceDirectory.getName());
    }

    for (File inputFile : files) {
      if (inputFile.isDirectory()) {
        induceClasses(inputFile);
        continue;
      }

      String className = parseClassName(inputFile.getPath());
      if (className == null) {
        continue; // Not a java file, continue
      }

      Class<?> klass = loadClass(classLoader, className);
      String fileName = getOutputFileName(klass);
      File outputFile = new File(fileName);
      outputFile.getParentFile().mkdirs();
      try (PrintWriter writer = new PrintWriter(fileName, encoding)) {
        if (klass.isInterface()) {
          writer.println(reflectData.getProtocol(klass).toString(true));
        } else {
          writer.println(reflectData.getSchema(klass).toString(true));
        }
      } catch (AIngleRuntimeException e) {
        throw new MojoExecutionException("Failed to resolve schema or protocol for class " + klass.getCanonicalName(),
            e);
      } catch (Exception e) {
        throw new MojoExecutionException("Failed to write output file for class " + klass.getCanonicalName(), e);
      }
    }
  }

  private String parseClassName(String fileName) {
    String indentifier = "java" + File.separator;
    int index = fileName.lastIndexOf(indentifier);
    String namespacedFileName = fileName.substring(index + indentifier.length());
    if (!namespacedFileName.endsWith(".java")) {
      return null;
    }

    return namespacedFileName.replace(File.separator, ".").replaceFirst("\\.java$", "");
  }

  private String getOutputFileName(Class klass) {
    String filename = aingleOutputDirectory.getPath() + File.separator + klass.getName().replace(".", File.separator);
    if (klass.isInterface()) {
      return filename.concat(".avpr");
    } else {
      return filename.concat(".ain");
    }
  }

  private ReflectData getReflectData() throws MojoExecutionException {
    if (reflectDataImplementation == null) {
      return allowNull ? ReflectData.AllowNull.get() : ReflectData.get();
    }

    try {
      Constructor<? extends ReflectData> constructor = loadClass(classLoader, reflectDataImplementation)
          .asSubclass(ReflectData.class).getConstructor();
      constructor.setAccessible(true);
      return constructor.newInstance();
    } catch (Exception e) {
      throw new MojoExecutionException(String.format(
          "Could not load ReflectData custom implementation %s. Make sure that it has a no-args constructor",
          reflectDataImplementation), e);
    }
  }

  private Class<?> loadClass(ClassLoader classLoader, String className) throws MojoExecutionException {
    try {
      return classLoader.loadClass(className);
    } catch (ClassNotFoundException e) {
      throw new MojoExecutionException("Failed to load class " + className, e);
    }
  }

  private ClassLoader getClassLoader() throws MojoExecutionException {
    ClassLoader classLoader;

    try {
      List<String> classpathElements = project.getRuntimeClasspathElements();
      if (null == classpathElements) {
        return Thread.currentThread().getContextClassLoader();
      }
      URL[] urls = new URL[classpathElements.size()];

      for (int i = 0; i < classpathElements.size(); ++i) {
        urls[i] = new File(classpathElements.get(i)).toURI().toURL();
      }
      classLoader = new URLClassLoader(urls, getClass().getClassLoader());
    } catch (Exception e) {
      throw new MojoExecutionException("Failed to obtain ClassLoader", e);
    }

    return classLoader;
  }
}
