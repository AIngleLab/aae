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

import static org.junit.Assert.*;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.aingle.Schema;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.io.Encoder;
import org.apache.aingle.io.EncoderFactory;
import org.apache.aingle.generic.GenericDatumWriter;
import org.apache.aingle.mapred.AIngleWrapper;
import org.junit.Test;

public class TestAIngleKeyDeserializer {
  @Test
  public void testDeserialize() throws IOException {
    // Create a deserializer.
    Schema writerSchema = Schema.create(Schema.Type.STRING);
    Schema readerSchema = Schema.create(Schema.Type.STRING);
    ClassLoader classLoader = this.getClass().getClassLoader();
    AIngleKeyDeserializer<CharSequence> deserializer = new AIngleKeyDeserializer<>(writerSchema, readerSchema, classLoader);

    // Check the schemas.
    assertEquals(writerSchema, deserializer.getWriterSchema());
    assertEquals(readerSchema, deserializer.getReaderSchema());

    // Write some records to deserialize.
    DatumWriter<CharSequence> datumWriter = new GenericDatumWriter<>(writerSchema);
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    Encoder encoder = EncoderFactory.get().binaryEncoder(outputStream, null);
    datumWriter.write("record1", encoder);
    datumWriter.write("record2", encoder);
    encoder.flush();

    // Deserialize the records.
    ByteArrayInputStream inputStream = new ByteArrayInputStream(outputStream.toByteArray());
    deserializer.open(inputStream);
    AIngleWrapper<CharSequence> record = null;

    record = deserializer.deserialize(record);
    assertEquals("record1", record.datum().toString());

    record = deserializer.deserialize(record);
    assertEquals("record2", record.datum().toString());

    deserializer.close();
  }
}
