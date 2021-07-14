/*

 */
package org.apache.aingle.ipc;

/**
 * An instrumentation API for RPC metadata. Each of these methods is invoked at
 * key points during the RPC exchange. Additionally, path-based
 * <em>metadata</em> that is passed along with the RPC call and can be set or
 * queried by subsequent instrumentation points.
 */
public class RPCPlugin {

  /**
   * Called on the client before the initial RPC handshake to setup any handshake
   * metadata for this plugin
   * 
   * @param context the handshake rpc context
   */
  public void clientStartConnect(RPCContext context) {
  }

  /**
   * Called on the server during the RPC handshake
   * 
   * @param context the handshake rpc context
   */
  public void serverConnecting(RPCContext context) {
  }

  /**
   * Called on the client after the initial RPC handshake
   * 
   * @param context the handshake rpc context
   */
  public void clientFinishConnect(RPCContext context) {
  }

  /**
   * This method is invoked at the client before it issues the RPC call.
   * 
   * @param context the per-call rpc context (in/out parameter)
   */
  public void clientSendRequest(RPCContext context) {
  }

  /**
   * This method is invoked at the RPC server when the request is received, but
   * before the call itself is executed
   * 
   * @param context the per-call rpc context (in/out parameter)
   */
  public void serverReceiveRequest(RPCContext context) {
  }

  /**
   * This method is invoked at the server before the response is executed, but
   * before the response has been formulated
   * 
   * @param context the per-call rpc context (in/out parameter)
   */
  public void serverSendResponse(RPCContext context) {
  }

  /**
   * This method is invoked at the client after the call is executed, and after
   * the client receives the response
   * 
   * @param context the per-call rpc context
   */
  public void clientReceiveResponse(RPCContext context) {
  }

}
