/*

 */

package org.apache.aingle.ipc;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.aingle.AIngleRuntimeException;

/** An {@link HttpServlet} that responds to AIngle RPC requests. */
public class ResponderServlet extends HttpServlet {
  private Responder responder;

  public ResponderServlet(Responder responder) throws IOException {
    this.responder = responder;
  }

  @Override
  protected void doPost(HttpServletRequest request, HttpServletResponse response) throws IOException, ServletException {
    response.setContentType(HttpTransceiver.CONTENT_TYPE);
    List<ByteBuffer> requestBufs = HttpTransceiver.readBuffers(request.getInputStream());
    try {
      List<ByteBuffer> responseBufs = responder.respond(requestBufs);
      response.setContentLength(HttpTransceiver.getLength(responseBufs));
      HttpTransceiver.writeBuffers(responseBufs, response.getOutputStream());
    } catch (AIngleRuntimeException e) {
      throw new ServletException(e);
    }
  }
}
