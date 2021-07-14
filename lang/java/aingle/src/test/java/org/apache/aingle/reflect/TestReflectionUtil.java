/*

 */
package org.apache.aingle.reflect;

import java.io.IOException;
import java.io.InputStream;

import org.junit.Test;

public class TestReflectionUtil {

  @Test
  public void testUnsafeUtil() {
    new Tester().checkUnsafe();
  }

  @Test
  public void testUnsafeWhenNotExists() throws Exception {
    ClassLoader cl = new NoUnsafe();
    Class<?> testerClass = cl.loadClass(Tester.class.getName());
    testerClass.getDeclaredMethod("checkUnsafe").invoke(testerClass.getDeclaredConstructor().newInstance());
  }

  public static final class Tester {
    public Tester() {
    }

    public void checkUnsafe() {
      ReflectionUtil.getFieldAccess();
    }

  }

  private static final class NoUnsafe extends ClassLoader {
    private ClassLoader parent = TestReflectionUtil.class.getClassLoader();

    @Override
    public java.lang.Class<?> loadClass(String name) throws ClassNotFoundException {
      Class<?> clazz = findLoadedClass(name);
      if (clazz != null) {
        return clazz;
      }
      if ("sun.misc.Unsafe".equals(name)) {
        throw new ClassNotFoundException(name);
      }
      if (!name.startsWith("org.apache.aingle.")) {
        return parent.loadClass(name);
      }

      InputStream data = parent.getResourceAsStream(name.replace('.', '/') + ".class");
      byte[] buf = new byte[10240]; // big enough, too lazy to loop
      int size;
      try {
        size = data.read(buf);
      } catch (IOException e) {
        throw new ClassNotFoundException();
      }
      clazz = defineClass(name, buf, 0, size);
      resolveClass(clazz);
      return clazz;
    }

  }
}
