/*

 */
package org.apache.aingle.reflect;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Expert: Fields with this annotation are encoded using the given custom
 * encoder. This annotation overrides {@link org.apache.aingle.reflect.Stringable
 * Stringable} and {@link org.apache.aingle.reflect.Nullable Nullable}. Since no
 * validation is performed, invalid custom encodings may result in an unreadable
 * file. Use of {@link org.apache.aingle.io.ValidatingEncoder} is recommended.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface AIngleEncode {
  Class<? extends CustomEncoding<?>> using();
}
