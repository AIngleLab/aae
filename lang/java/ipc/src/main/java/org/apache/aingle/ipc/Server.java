/*

 */

package org.apache.aingle.ipc;

/** A server listening on a port. */
public interface Server {
  /** The port this server runs on. */
  int getPort();

  /** Start this server. */
  void start();

  /** Stop this server. */
  void close();

  /** Wait for this server to exit. */
  void join() throws InterruptedException;

}
