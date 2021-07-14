/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 * 
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef AINGLE_RESOLVER_H
#define AINGLE_RESOLVER_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/schema.h>
#include <aingle/value.h>

/*
 * A <i>resolved value</i> is a special kind of value that knows how to
 * implement AIngle's schema resolution rules to translate between a
 * writer schema and a reader schema.  A resolved value doesn't store or
 * process data itself; instead, it wraps an existing value instance.
 *
 * There are two resolved value classes.  In the first (@ref
 * aingle_resolved_writer_t), the resolved value is an instance of the
 * writer schema, and wraps an instance of the reader schema.  This is
 * used, for instance, when reading from an AIngle data file; you want the
 * end result to be a reader schema value, and the resolved value allows
 * the file reader to ignore the schema resolution and simply fill in
 * the values of the writer schema.  You can only set the values of a
 * resolved writer; you must use the original wrapped value to read.
 *
 * With other class (@ref aingle_resolved_reader_t), the resolved value is
 * an instance of the reader schema, and wraps an instance of the writer
 * schema.  This is used when resolving an existing AIngle value to
 * another schema; you've already got the value in the original (writer)
 * schema, and want to transparently treat it as if it were an instance
 * of the new (reader) schema.  You can only read the values of a
 * resolved reader; you must use the original wrapped value to write.
 *
 * For both classes, the “self” pointer of the resolved value is an
 * aingle_value_t pointer, which points at the wrapped value.
 */


/**
 * Create a new resolved writer implementation for the given writer and
 * reader schemas.
 */

aingle_value_iface_t *
aingle_resolved_writer_new(aingle_schema_t writer_schema,
			 aingle_schema_t reader_schema);

/**
 * Creates a new resolved writer value.
 */

int
aingle_resolved_writer_new_value(aingle_value_iface_t *iface,
			       aingle_value_t *value);

/**
 * Sets the wrapped value for a resolved writer.  This must be an
 * instance of the reader schema.  We create our own reference to the
 * destination value.
 */

void
aingle_resolved_writer_set_dest(aingle_value_t *resolved,
			      aingle_value_t *dest);


/**
 * Clears the wrapped value for a resolved writer.
 */

void
aingle_resolved_writer_clear_dest(aingle_value_t *resolved);


/**
 * Create a new resolved reader implementation for the given writer and
 * reader schemas.
 */

aingle_value_iface_t *
aingle_resolved_reader_new(aingle_schema_t writer_schema,
			 aingle_schema_t reader_schema);

/**
 * Creates a new resolved reader value.
 */

int
aingle_resolved_reader_new_value(aingle_value_iface_t *iface,
			       aingle_value_t *value);

/**
 * Sets the wrapped value for a resolved reader.  This must be an
 * instance of the reader schema.  We create our own reference to the
 * source value.
 */

void
aingle_resolved_reader_set_source(aingle_value_t *resolved,
				aingle_value_t *dest);


/**
 * Clears the wrapped value for a resolved reader.
 */

void
aingle_resolved_reader_clear_source(aingle_value_t *resolved);

CLOSE_EXTERN
#endif
