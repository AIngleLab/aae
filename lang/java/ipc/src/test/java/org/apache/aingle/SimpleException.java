/*

 */
package org.apache.aingle;

/**
 * This should be a static nested class in TestProtocolReflect, but that breaks
 * CheckStyle (http://jira.codehaus.org/browse/MPCHECKSTYLE-20).
 */
public class SimpleException extends Exception {
  SimpleException() {
  }

  SimpleException(String message) {
    super(message);
  }
}
