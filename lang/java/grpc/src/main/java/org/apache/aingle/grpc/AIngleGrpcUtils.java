/*

 */

package org.apache.aingle.grpc;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.Protocol;

import java.io.InputStream;
import java.util.logging.Level;
import java.util.logging.Logger;

import io.grpc.KnownLength;

/** Utility methods for using AIngle IDL and serialization with gRPC. */
public final class AIngleGrpcUtils {
  private static final Logger LOG = Logger.getLogger(AIngleGrpcUtils.class.getName());

  private AIngleGrpcUtils() {
  }

  /**
   * Provides a a unique gRPC service name for AIngle RPC interface or its subclass
   * Callback Interface.
   *
   * @param iface AIngle RPC interface.
   * @return unique service name for gRPC.
   */
  public static String getServiceName(Class iface) {
    Protocol protocol = getProtocol(iface);
    return protocol.getNamespace() + "." + protocol.getName();
  }

  /**
   * Gets the {@link Protocol} from the AIngle Interface.
   */
  public static Protocol getProtocol(Class iface) {
    try {
      Protocol p = (Protocol) (iface.getDeclaredField("PROTOCOL").get(null));
      return p;
    } catch (NoSuchFieldException e) {
      throw new AIngleRuntimeException("Not a Specific protocol: " + iface);
    } catch (IllegalAccessException e) {
      throw new AIngleRuntimeException(e);
    }
  }

  /**
   * Skips any unread bytes from InputStream and closes it.
   */
  static void skipAndCloseQuietly(InputStream stream) {
    try {
      if (stream instanceof KnownLength && stream.available() > 0) {
        stream.skip(stream.available());
      } else {
        // don't expect this for an inputStream provided by gRPC but just to be on safe
        // side.
        byte[] skipBuffer = new byte[4096];
        while (true) {
          int read = stream.read(skipBuffer);
          if (read < skipBuffer.length) {
            break;
          }
        }
      }
      stream.close();
    } catch (Exception e) {
      LOG.log(Level.WARNING, "failed to skip/close the input stream, may cause memory leak", e);
    }
  }
}
