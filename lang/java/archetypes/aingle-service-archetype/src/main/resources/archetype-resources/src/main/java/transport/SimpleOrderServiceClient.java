#set( $symbol_pound = '#' )
#set( $symbol_dollar = '$' )
#set( $symbol_escape = '\' )
/*

 */

package ${package}.transport;

import java.io.IOException;
import java.net.InetSocketAddress;

import org.apache.aingle.ipc.Transceiver;
import org.apache.aingle.ipc.netty.NettyTransceiver;
import org.apache.aingle.ipc.specific.SpecificRequestor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import ${package}.service.Confirmation;
import ${package}.service.Order;
import ${package}.service.OrderFailure;
import ${package}.service.OrderProcessingService;

/**
 * {@code SimpleOrderServiceClient} is a basic client for the Netty backed {@link OrderProcessingService}
 * implementation.
 */
public class SimpleOrderServiceClient implements OrderProcessingService {

  private static final Logger LOG = LoggerFactory.getLogger(SimpleOrderServiceEndpoint.class);

  private InetSocketAddress endpointAddress;

  private Transceiver transceiver;

  private OrderProcessingService service;

  public SimpleOrderServiceClient(InetSocketAddress endpointAddress) {
    this.endpointAddress = endpointAddress;
  }

  public synchronized void start() throws IOException {
    if (LOG.isInfoEnabled()) {
      LOG.info("Starting Simple Ordering Netty client on '{}'", endpointAddress);
    }
    transceiver = new NettyTransceiver(endpointAddress);
    service = SpecificRequestor.getClient(OrderProcessingService.class, transceiver);
  }

  public void stop() throws IOException {
    if (LOG.isInfoEnabled()) {
      LOG.info("Stopping Simple Ordering Netty client on '{}'", endpointAddress);
    }
    if (transceiver != null && transceiver.isConnected()) {
      transceiver.close();
    }
  }

  @Override
  public Confirmation submitOrder(Order order) throws OrderFailure {
    return service.submitOrder(order);
  }

}
