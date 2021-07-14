/*

 */

package org.apache.aingle;

import org.apache.aingle.Schema.Field;

import java.util.ArrayList;
import java.util.List;

/** AIngle exception in case of missing fields. */
public class AIngleMissingFieldException extends AIngleRuntimeException {
  private List<Field> chainOfFields = new ArrayList<>(8);

  public AIngleMissingFieldException(String message, Field field) {
    super(message);
    chainOfFields.add(field);
  }

  public void addParentField(Field field) {
    chainOfFields.add(field);
  }

  @Override
  public String toString() {
    StringBuilder result = new StringBuilder();
    for (Field field : chainOfFields) {
      result.insert(0, " --> " + field.name());
    }
    return "Path in schema:" + result;
  }
}
