/*

 */
package org.apache.aingle.reflect;

import java.lang.annotation.ElementType;
import java.lang.annotation.Repeatable;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Adds the given name and space as an alias to the schema. AIngle files of this
 * schema can be read into classes named by the alias.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ ElementType.TYPE, ElementType.FIELD })
@Repeatable(AIngleAliases.class)
public @interface AIngleAlias {
  String NULL = "NOT A VALID NAMESPACE";

  String alias();

  String space() default NULL;
}
