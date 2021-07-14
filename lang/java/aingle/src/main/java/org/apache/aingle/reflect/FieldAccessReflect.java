/*

 */
package org.apache.aingle.reflect;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.io.Decoder;
import org.apache.aingle.io.Encoder;

import java.io.IOException;
import java.lang.reflect.Field;

class FieldAccessReflect extends FieldAccess {

  @Override
  protected FieldAccessor getAccessor(Field field) {
    AIngleEncode enc = field.getAnnotation(AIngleEncode.class);
    if (enc != null)
      try {
        return new ReflectionBasesAccessorCustomEncoded(field, enc.using().getDeclaredConstructor().newInstance());
      } catch (Exception e) {
        throw new AIngleRuntimeException("Could not instantiate custom Encoding");
      }
    return new ReflectionBasedAccessor(field);
  }

  private static class ReflectionBasedAccessor extends FieldAccessor {
    protected final Field field;
    private boolean isStringable;
    private boolean isCustomEncoded;

    public ReflectionBasedAccessor(Field field) {
      this.field = field;
      this.field.setAccessible(true);
      isStringable = field.isAnnotationPresent(Stringable.class);
      isCustomEncoded = field.isAnnotationPresent(AIngleEncode.class);
    }

    @Override
    public String toString() {
      return field.getName();
    }

    @Override
    public Object get(Object object) throws IllegalAccessException {
      return field.get(object);
    }

    @Override
    public void set(Object object, Object value) throws IllegalAccessException, IOException {
      field.set(object, value);
    }

    @Override
    protected Field getField() {
      return field;
    }

    @Override
    protected boolean isStringable() {
      return isStringable;
    }

    @Override
    protected boolean isCustomEncoded() {
      return isCustomEncoded;
    }
  }

  private static final class ReflectionBasesAccessorCustomEncoded extends ReflectionBasedAccessor {

    private CustomEncoding<?> encoding;

    public ReflectionBasesAccessorCustomEncoded(Field f, CustomEncoding<?> encoding) {
      super(f);
      this.encoding = encoding;
    }

    @Override
    protected void read(Object object, Decoder in) throws IOException {
      try {
        field.set(object, encoding.read(in));
      } catch (IllegalAccessException e) {
        throw new AIngleRuntimeException(e);
      }
    }

    @Override
    protected void write(Object object, Encoder out) throws IOException {
      try {
        encoding.write(field.get(object), out);
      } catch (IllegalAccessException e) {
        throw new AIngleRuntimeException(e);
      }
    }

    @Override
    protected boolean isCustomEncoded() {
      return true;
    }

    @Override
    protected boolean supportsIO() {
      return true;
    }
  }
}
