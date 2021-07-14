/*

 */
package org.apache.aingle.file;

import static org.junit.Assert.assertTrue;

import java.io.IOException;

import org.junit.Test;

public class TestZstandardCodec {

  @Test
  public void testZstandardToStringAndName() throws IOException {
    Codec codec = CodecFactory.zstandardCodec(3).createInstance();
    assertTrue(codec instanceof ZstandardCodec);
    assertTrue(codec.getName().equals("zstandard"));
    assertTrue(codec.toString().equals("zstandard[3]"));
  }
}
