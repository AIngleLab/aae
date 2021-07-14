/*

 */
package org.apache.aingle.ipc.netty;

import org.apache.aingle.ipc.Transceiver;

import org.junit.Assert;
import org.junit.Test;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;

import io.netty.channel.socket.SocketChannel;

/**
 * This is a very specific test that verifies that if the NettyTransceiver fails
 * to connect it cleans up the netty channel that it has created.
 */
public class TestNettyTransceiverWhenFailsToConnect {
  SocketChannel channel = null;

  @Test(expected = IOException.class)
  public void testNettyTransceiverReleasesNettyChannelOnFailingToConnect() throws Exception {
    try (ServerSocket serverSocket = new ServerSocket(0)) {
      try (Transceiver t = new NettyTransceiver(new InetSocketAddress(serverSocket.getLocalPort()), 1, c -> {
        channel = c;
      })) {
        Assert.fail("should have thrown an exception");
      }
    } finally {
      Assert.assertTrue("Channel not shut down", channel == null || channel.isShutdown());
    }
  }
}
