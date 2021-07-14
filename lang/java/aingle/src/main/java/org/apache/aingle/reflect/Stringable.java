/*

 */
package org.apache.aingle.reflect;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Declares that a class or field should be represented by an AIngle string. It's
 * {@link Object#toString()} method will be used to convert it to a string, and
 * its single String parameter constructor will be used to create instances.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ ElementType.TYPE, ElementType.FIELD })
@Documented
public @interface Stringable {
}
