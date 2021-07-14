/**

 */
package org.apache.aingle;

import java.nio.ByteBuffer;

public class ByteBufferRecord {

  private ByteBuffer payload;
  private TypeEnum tp;

  public ByteBufferRecord() {
  }

  public ByteBuffer getPayload() {
    return payload;
  }

  public void setPayload(ByteBuffer payload) {
    this.payload = payload;
  }

  public TypeEnum getTp() {
    return tp;
  }

  public void setTp(TypeEnum tp) {
    this.tp = tp;
  }

  @Override
  public boolean equals(Object ob) {
    if (this == ob)
      return true;
    if (!(ob instanceof ByteBufferRecord))
      return false;
    ByteBufferRecord that = (ByteBufferRecord) ob;
    if (this.getPayload() == null)
      return that.getPayload() == null;
    if (!this.getPayload().equals(that.getPayload()))
      return false;
    if (this.getTp() == null)
      return that.getTp() == null;
    return this.getTp().equals(that.getTp());
  }

  @Override
  public int hashCode() {
    return this.payload.hashCode();
  }
}
