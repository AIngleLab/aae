/*

 */
package org.apache.aingle.reflect;

import java.lang.reflect.Field;

abstract class FieldAccess {

  protected abstract FieldAccessor getAccessor(Field field);

}
