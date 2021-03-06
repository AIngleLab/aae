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
import org.apache.aingle.generic.GenericData;
import org.apache.aingle.io.BinaryData;
import org.apache.aingle.mapred.AIngleKey;
import org.apache.aingle.mapreduce.AIngleJob;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.io.RawComparator;

/**
 * The {@link org.apache.hadoop.io.RawComparator} used by jobs configured with
 * {@link org.apache.aingle.mapreduce.AIngleJob}.
 *
 * <p>
 * Compares AIngleKeys output from the map phase for sorting.
 * </p>
 */
public class AIngleKeyComparator<T> extends Configured implements RawComparator<AIngleKey<T>> {
  /** The schema of the AIngle data in the key to compare. */
  private Schema mSchema;
  private GenericData mDataModel;

  /** {@inheritDoc} */
  @Override
  public void setConf(Configuration conf) {
    super.setConf(conf);
    if (null != conf) {
      // The MapReduce framework will be using this comparator to sort AIngleKey objects
      // output from the map phase, so use the schema defined for the map output key
      // and the data model non-raw compare() implementation.
      mSchema = AIngleJob.getMapOutputKeySchema(conf);
      mDataModel = AIngleSerialization.createDataModel(conf);
    }
  }

  /** {@inheritDoc} */
  @Override
  public int compare(byte[] b1, int s1, int l1, byte[] b2, int s2, int l2) {
    return BinaryData.compare(b1, s1, b2, s2, mSchema);
  }

  /** {@inheritDoc} */
  @Override
  public int compare(AIngleKey<T> x, AIngleKey<T> y) {
    return mDataModel.compare(x.datum(), y.datum(), mSchema);
  }
}
