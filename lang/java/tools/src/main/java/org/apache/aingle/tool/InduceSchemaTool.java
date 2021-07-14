/*

 */
package org.apache.aingle.tool;

import java.io.File;
import java.io.InputStream;
import java.io.PrintStream;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.List;

import org.apache.aingle.reflect.ReflectData;

/**
 * Utility to induce a schema from a class or a protocol from an interface.
 */
public class InduceSchemaTool implements Tool {

  @Override
  public int run(InputStream in, PrintStream out, PrintStream err, List<String> args) throws Exception {
    if (args.size() == 0 || args.size() > 2) {
      System.err.println("Usage: [colon-delimited-classpath] classname");
      return 1;
    }
    ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
    String className;
    if (args.size() == 2) {
      String classpaths = args.get(0);
      className = args.get(1);
      if (!classpaths.isEmpty()) {
        String[] paths = args.get(0).split(":");
        URL[] urls = new URL[paths.length];
        for (int i = 0; i < paths.length; ++i) {
          urls[i] = new File(paths[i]).toURI().toURL();
        }
        classLoader = URLClassLoader.newInstance(urls, classLoader);
      }
    } else {
      className = args.get(0);
    }

    Class<?> klass = classLoader.loadClass(className);
    if (klass.isInterface()) {
      System.out.println(ReflectData.get().getProtocol(klass).toString(true));
    } else {
      System.out.println(ReflectData.get().getSchema(klass).toString(true));
    }
    return 0;
  }

  @Override
  public String getName() {
    return "induce";
  }

  @Override
  public String getShortDescription() {
    return "Induce schema/protocol from Java class/interface via reflection.";
  }
}
