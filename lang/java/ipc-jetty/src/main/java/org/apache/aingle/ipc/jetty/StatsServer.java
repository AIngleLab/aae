package org.apache.aingle.ipc.jetty;

import org.apache.aingle.ipc.stats.StatsPlugin;
import org.apache.aingle.ipc.stats.StatsServlet;
/*

 */
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.servlet.ServletHandler;
import org.eclipse.jetty.servlet.ServletHolder;

/* This is a server that displays live information from a StatsPlugin.
 *
 *  Typical usage is as follows:
 *    StatsPlugin plugin = new StatsPlugin();
 *    requestor.addPlugin(plugin);
 *    StatsServer server = new StatsServer(plugin, 8080);
 *
 *  */
public class StatsServer {
  Server httpServer;
  StatsPlugin plugin;

  /*
   * Start a stats server on the given port, responsible for the given plugin.
   */
  public StatsServer(StatsPlugin plugin, int port) throws Exception {
    this.httpServer = new Server(port);
    this.plugin = plugin;

    ServletHandler handler = new ServletHandler();
    httpServer.setHandler(handler);
    handler.addServletWithMapping(new ServletHolder(new StaticServlet()), "/");

    handler.addServletWithMapping(new ServletHolder(new StatsServlet(plugin)), "/");

    httpServer.start();
  }

  /* Stops this server. */
  public void stop() throws Exception {
    this.httpServer.stop();
  }
}
