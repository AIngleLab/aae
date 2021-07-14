/*

 */
package org.apache.aingle.specific;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Declares the size of implementations of
 * {@link org.apache.aingle.generic.GenericFixed GenericFixed}.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ ElementType.TYPE })
@Documented
public @interface FixedSize {
  /** The declared size of instances of classes with this annotation. */
  int value();
}
