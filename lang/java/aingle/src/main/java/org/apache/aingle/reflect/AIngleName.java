/*

 */
package org.apache.aingle.reflect;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Sets the ainglename for this java field. When reading into this class, a
 * reflectdatumreader looks for a schema field with the ainglename.
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface AIngleName {
  String value();
}
