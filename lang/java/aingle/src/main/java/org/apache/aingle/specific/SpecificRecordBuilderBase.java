/*

 */
package org.apache.aingle.specific;

import org.apache.aingle.Schema;
import org.apache.aingle.data.RecordBuilderBase;

/**
 * Abstract base class for specific RecordBuilder implementations. Not
 * thread-safe.
 */
abstract public class SpecificRecordBuilderBase<T extends SpecificRecord> extends RecordBuilderBase<T> {

  /**
   * Creates a SpecificRecordBuilderBase for building records of the given type.
   * 
   * @param schema the schema associated with the record class.
   */
  protected SpecificRecordBuilderBase(Schema schema) {
    super(schema, SpecificData.getForSchema(schema));
  }

  /**
   * Creates a SpecificRecordBuilderBase for building records of the given type.
   * 
   * @param schema the schema associated with the record class.
   * @param model  the SpecificData associated with the specific record class
   */
  protected SpecificRecordBuilderBase(Schema schema, SpecificData model) {
    super(schema, model);
  }

  /**
   * SpecificRecordBuilderBase copy constructor.
   * 
   * @param other SpecificRecordBuilderBase instance to copy.
   */
  protected SpecificRecordBuilderBase(SpecificRecordBuilderBase<T> other) {
    super(other, other.data());
  }

  /**
   * Creates a SpecificRecordBuilderBase by copying an existing record instance.
   * 
   * @param other the record instance to copy.
   */
  protected SpecificRecordBuilderBase(T other) {
    super(other.getSchema(), SpecificData.getForSchema(other.getSchema()));
  }
}
