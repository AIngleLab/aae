/*

 */

package org.apache.aingle.file;

import static java.nio.charset.StandardCharsets.UTF_8;
import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.apache.aingle.file.codec.CustomCodec;
import org.junit.Test;

public class TestCustomCodec {

  @Test
  public void testCustomCodec() {
    CustomCodec customCodec = new CustomCodec();
    Codec snappyCodec = new SnappyCodec.Option().createInstance();
    assertTrue(customCodec.equals(new CustomCodec()));
    assertFalse(customCodec.equals(snappyCodec));

    String testString = "Testing 123";
    ByteBuffer original = ByteBuffer.allocate(testString.getBytes(UTF_8).length);
    original.put(testString.getBytes(UTF_8));
    original.rewind();
    ByteBuffer decompressed = null;
    try {
      ByteBuffer compressed = customCodec.compress(original);
      compressed.rewind();
      decompressed = customCodec.decompress(compressed);
    } catch (IOException e) {
      e.printStackTrace();
    }

    assertEquals(testString, new String(decompressed.array(), UTF_8));

  }

}
