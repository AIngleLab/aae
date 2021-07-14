/*

 */

package org.apache.aingle.specific;

import java.io.Externalizable;
import java.io.ObjectOutput;
import java.io.ObjectInput;
import java.io.IOException;

import org.apache.aingle.AIngleRemoteException;
import org.apache.aingle.Schema;

/** Base class for specific exceptions. */
public abstract class SpecificExceptionBase extends AIngleRemoteException implements SpecificRecord, Externalizable {

  public SpecificExceptionBase() {
    super();
  }

  public SpecificExceptionBase(Throwable value) {
    super(value);
  }

  public SpecificExceptionBase(Object value) {
    super(value);
  }

  public SpecificExceptionBase(Object value, Throwable cause) {
    super(value, cause);
  }

  @Override
  public abstract Schema getSchema();

  @Override
  public abstract Object get(int field);

  @Override
  public abstract void put(int field, Object value);

  @Override
  public boolean equals(Object that) {
    if (that == this)
      return true; // identical object
    if (!(that instanceof SpecificExceptionBase))
      return false; // not a record
    if (this.getClass() != that.getClass())
      return false; // not same schema
    return SpecificData.get().compare(this, that, this.getSchema()) == 0;
  }

  @Override
  public int hashCode() {
    return SpecificData.get().hashCode(this, this.getSchema());
  }

  @Override
  public abstract void writeExternal(ObjectOutput out) throws IOException;

  @Override
  public abstract void readExternal(ObjectInput in) throws IOException;

}
