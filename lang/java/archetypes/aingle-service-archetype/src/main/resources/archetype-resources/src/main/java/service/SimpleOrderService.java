#set( $symbol_pound = '#' )
#set( $symbol_dollar = '$' )
#set( $symbol_escape = '\' )
/*

 */

package ${package}.service;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * {@code SimpleOrderService} is a simple example implementation of an AIngle service generated from the
 * order-service.avpr protocol definition.
 */
public class SimpleOrderService implements OrderProcessingService {

  private Logger log = LoggerFactory.getLogger(SimpleOrderService.class);

  @Override
  public Confirmation submitOrder(Order order) throws OrderFailure {
    log.info("Received order for '{}' items from customer with id '{}'",
      new Object[] {order.getOrderItems().size(), order.getCustomerId()});

    long estimatedCompletion = System.currentTimeMillis() + (5 * 60 * 60);
    return Confirmation.newBuilder().setCustomerId(order.getCustomerId()).setEstimatedCompletion(estimatedCompletion)
      .setOrderId(order.getOrderId()).build();
  }
}
