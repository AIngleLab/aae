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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

package org.apache.aingle.hadoop.io;

import org.apache.aingle.Schema;

/**
 * Converts a Java object into an AIngle datum.
 *
 * @param <INPUT>  The type of the input Java object to convert.
 * @param <OUTPUT> The type of the AIngle datum to convert to.
 */
public abstract class AIngleDatumConverter<INPUT, OUTPUT> {
  public abstract OUTPUT convert(INPUT input);

  /**
   * Gets the writer schema that should be used to serialize the output AIngle
   * datum.
   *
   * @return The writer schema for the output AIngle datum.
   */
  public abstract Schema getWriterSchema();
}
