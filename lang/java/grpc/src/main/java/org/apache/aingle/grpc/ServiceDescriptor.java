/*

 */

package org.apache.aingle.grpc;

import org.apache.aingle.Protocol;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import io.grpc.MethodDescriptor;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/** Descriptor for a gRPC service based on a AIngle interface. */
class ServiceDescriptor {

  // cache for service descriptors.
  private static final ConcurrentMap<String, ServiceDescriptor> SERVICE_DESCRIPTORS = new ConcurrentHashMap<>();
  private final String serviceName;
  private final Protocol protocol;
  // cache for method descriptors.
  private final ConcurrentMap<String, MethodDescriptor<Object[], Object>> methods = new ConcurrentHashMap<>();

  private ServiceDescriptor(Class iface, String serviceName) {
    this.serviceName = serviceName;
    this.protocol = AIngleGrpcUtils.getProtocol(iface);
  }

  /**
   * Creates a Service Descriptor.
   *
   * @param iface AIngle RPC interface.
   */
  public static ServiceDescriptor create(Class iface) {
    String serviceName = AIngleGrpcUtils.getServiceName(iface);
    return SERVICE_DESCRIPTORS.computeIfAbsent(serviceName, key -> new ServiceDescriptor(iface, serviceName));
  }

  /**
   * provides name of the service.
   */
  public String getServiceName() {
    return serviceName;
  }

  /**
   * Provides a gRPC {@link MethodDescriptor} for a RPC method/message of AIngle
   * {@link Protocol}.
   *
   * @param methodType gRPC type for the method.
   * @return a {@link MethodDescriptor}
   */
  public MethodDescriptor<Object[], Object> getMethod(String methodName, MethodDescriptor.MethodType methodType) {
    return methods.computeIfAbsent(methodName,
        key -> MethodDescriptor.<Object[], Object>newBuilder()
            .setFullMethodName(generateFullMethodName(serviceName, methodName)).setType(methodType)
            .setRequestMarshaller(new AIngleRequestMarshaller(protocol.getMessages().get(methodName)))
            .setResponseMarshaller(new AIngleResponseMarshaller(protocol.getMessages().get(methodName))).build());
  }
}
