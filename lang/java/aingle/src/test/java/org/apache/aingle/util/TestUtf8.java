/*

 */
package org.apache.aingle.util;

import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertEquals;

import java.nio.charset.StandardCharsets;

import org.junit.Test;

public class TestUtf8 {
  @Test
  public void testByteConstructor() throws Exception {
    byte[] bs = "Foo".getBytes(StandardCharsets.UTF_8);
    Utf8 u = new Utf8(bs);
    assertEquals(bs.length, u.getByteLength());
    for (int i = 0; i < bs.length; i++) {
      assertEquals(bs[i], u.getBytes()[i]);
    }
  }

  @Test
  public void testArrayReusedWhenLargerThanRequestedSize() {
    byte[] bs = "55555".getBytes(StandardCharsets.UTF_8);
    Utf8 u = new Utf8(bs);
    assertEquals(5, u.getByteLength());
    byte[] content = u.getBytes();
    u.setByteLength(3);
    assertEquals(3, u.getByteLength());
    assertSame(content, u.getBytes());
    u.setByteLength(4);
    assertEquals(4, u.getByteLength());
    assertSame(content, u.getBytes());
  }

  @Test
  public void testHashCodeReused() {
    assertEquals(97, new Utf8("a").hashCode());
    assertEquals(3904, new Utf8("zz").hashCode());
    assertEquals(122, new Utf8("z").hashCode());
    assertEquals(99162322, new Utf8("hello").hashCode());
    assertEquals(3198781, new Utf8("hell").hashCode());

    Utf8 u = new Utf8("a");
    assertEquals(97, u.hashCode());
    assertEquals(97, u.hashCode());

    u.set("a");
    assertEquals(97, u.hashCode());

    u.setByteLength(1);
    assertEquals(97, u.hashCode());
    u.setByteLength(2);
    assertNotEquals(97, u.hashCode());

    u.set("zz");
    assertEquals(3904, u.hashCode());
    u.setByteLength(1);
    assertEquals(122, u.hashCode());

    u.set("hello");
    assertEquals(99162322, u.hashCode());
    u.setByteLength(4);
    assertEquals(3198781, u.hashCode());

    u.set(new Utf8("zz"));
    assertEquals(3904, u.hashCode());
    u.setByteLength(1);
    assertEquals(122, u.hashCode());

    u.set(new Utf8("hello"));
    assertEquals(99162322, u.hashCode());
    u.setByteLength(4);
    assertEquals(3198781, u.hashCode());
  }
}
