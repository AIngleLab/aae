/*

 */
package org.apache.trevni;

import java.io.IOException;

/** File-level metadata. */
public class ColumnFileMetaData extends MetaData<ColumnFileMetaData> {

  static ColumnFileMetaData read(InputBuffer in) throws IOException {
    ColumnFileMetaData result = new ColumnFileMetaData();
    MetaData.read(in, result);
    return result;
  }

}
