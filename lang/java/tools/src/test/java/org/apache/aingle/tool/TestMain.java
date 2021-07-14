/*

 */
package org.apache.aingle.tool;

import static org.junit.Assert.fail;

import org.junit.Test;

public class TestMain {
  /** Make sure that tool descriptions fit in 80 characters. */
  @Test
  public void testToolDescriptionLength() {
    Main m = new Main();
    for (Tool t : m.tools.values()) {
      // System.out.println(t.getName() + ": " + t.getShortDescription().length());
      if (m.maxLen + 2 + t.getShortDescription().length() > 80) {
        fail("Tool description too long: " + t.getName());
      }
    }
  }

  /**
   * Make sure that the tool name is not too long, otherwise space for description
   * is too short because they are rebalanced in the CLI.
   */
  @Test
  public void testToolNameLength() {
    // 13 chosen for backwards compatibility
    final int MAX_NAME_LENGTH = 13;

    Main m = new Main();
    for (Tool t : m.tools.values()) {
      if (t.getName().length() > MAX_NAME_LENGTH) {
        fail("Tool name too long (" + t.getName().length() + "): " + t.getName() + ". Max length is: "
            + MAX_NAME_LENGTH);
      }
    }
  }
}
