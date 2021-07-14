/*

 */

package org.apache.aingle.ipc.jetty;

import java.net.URL;

import org.eclipse.jetty.servlet.DefaultServlet;
import org.eclipse.jetty.util.resource.Resource;

/**
 * Very simple servlet class capable of serving static files.
 */
public class StaticServlet extends DefaultServlet {
  private static final long serialVersionUID = 1L;

  @Override
  public Resource getResource(String pathInContext) {
    // Take only last slice of the URL as a filename, so we can adjust path.
    // This also prevents mischief like '../../foo.css'
    String[] parts = pathInContext.split("/");
    String filename = parts[parts.length - 1];

    URL resource = getClass().getClassLoader().getResource("org/apache/aingle/ipc/stats/static/" + filename);
    if (resource == null) {
      return null;
    }
    return Resource.newResource(resource);
  }
}
