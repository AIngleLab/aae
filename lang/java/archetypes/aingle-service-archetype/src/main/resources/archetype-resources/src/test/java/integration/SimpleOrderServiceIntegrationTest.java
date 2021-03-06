#set( $symbol_pound = '#' )
#set( $symbol_dollar = '$' )
#set( $symbol_escape = '\' )
/*

 */

package ${package}.integration;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;

import ${package}.transport.SimpleOrderServiceEndpoint;
import ${package}.transport.SimpleOrderServiceClient;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import ${package}.service.Confirmation;
import ${package}.service.Item;
import ${package}.service.Order;

/**
 * <code>SimpleOrderServiceIntegrationTest</code> runs as part of the Integration phase of the build and is
 * meant for end to end service testing.
 */
public class SimpleOrderServiceIntegrationTest {

  private static SimpleOrderServiceEndpoint service;
  private static SimpleOrderServiceClient client;

  @Test
  public void simpleRoundTripTest() throws Exception {
    Order simpleOrder = createOrder();
    Confirmation c = client.submitOrder(simpleOrder);

    assertEquals(c.getOrderId(), simpleOrder.getOrderId());
    assertEquals(c.getCustomerId(), simpleOrder.getCustomerId());
    assertTrue(c.getEstimatedCompletion() > 0);
  }

  @BeforeClass
  public static void setupTransport() throws Exception {
    InetSocketAddress endpointAddress = new InetSocketAddress("0.0.0.0", 12345);
    service = new SimpleOrderServiceEndpoint(endpointAddress);
    client = new SimpleOrderServiceClient(endpointAddress);

    service.start();
    client.start();
  }

  @AfterClass
  public static void shutdownTransport() throws Exception {
    client.stop();
    service.stop();
  }

  public Order createOrder() {
    return Order.newBuilder().setOrderId(1).setCustomerId(1).setOrderItems(createItems()).build();
  }

  public List<Item> createItems() {
    List<Item> items = new ArrayList<Item>();
    for (int x = 0; x < 5; x++)
      items.add(Item.newBuilder().setName("Item-" + x).setQuantity(x + 1).setSku(1230 + x).build());
    return items;
  }

}
