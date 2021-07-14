/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.apache.aingle.ipc.specific;

import static org.junit.Assert.fail;

import java.io.IOException;
import java.net.URL;

import org.apache.aingle.AIngleRuntimeException;
import org.apache.aingle.Protocol;
import org.apache.aingle.ipc.HttpTransceiver;
import org.junit.BeforeClass;
import org.junit.Test;

public class TestSpecificRequestor {
  public interface SampleSpecificProtocol {
    public static final Protocol PROTOCOL = Protocol.parse(
        "{\"protocol\":\"SampleSpecificProtocol\",\"namespace\":\"org.apache.aingle.ipc.specific\",\"types\":[],\"messages\":{}}");
  }

  static Object proxy;

  @BeforeClass
  public static void initializeProxy() throws Exception {
    HttpTransceiver client = new HttpTransceiver(new URL("http://localhost"));
    SpecificRequestor requestor = new SpecificRequestor(SampleSpecificProtocol.class, client);
    proxy = SpecificRequestor.getClient(SampleSpecificProtocol.class, requestor);
  }

  @Test
  public void testHashCode() throws IOException {
    try {
      proxy.hashCode();
    } catch (AIngleRuntimeException e) {
      fail(e.getMessage());
    }
  }

  @Test
  public void testEquals() throws IOException {
    try {
      proxy.equals(proxy);
    } catch (AIngleRuntimeException e) {
      fail(e.getMessage());
    }
  }

  @Test
  public void testToString() throws IOException {
    try {
      proxy.toString();
    } catch (AIngleRuntimeException e) {
      fail(e.getMessage());
    }
  }

}
