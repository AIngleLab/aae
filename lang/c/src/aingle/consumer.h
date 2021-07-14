/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.	 You may obtain a copy of the License at
 *
 * 
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef AINGLE_CONSUMER_H
#define AINGLE_CONSUMER_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/platform.h>
#include <stdlib.h>

#include <aingle/io.h>
#include <aingle/schema.h>


/*---------------------------------------------------------------------
 * Consumers
 */

/**
 * A <i>consumer</i> is an object that knows how to process AIngle data.
 * There are consumer methods for each type of AIngle data.  The
 * <code>aingle_consumer_t</code> struct is an abstract superclass, which
 * you don't instantiate directly.  Later in this file, we define
 * several consumer classes that know how to process AIngle data in
 * specific ways.
 *
 * For compound AIngle values (records, arrays, maps, and unions), the
 * consumer callbacks provide a nested consumer that should be used to
 * process subvalues.  Each consumer instance, including these
 * "subconsumers", contains a reference to the schema of the data that
 * it expects to process.  This means that the functions that produce
 * AIngle data (such as aingle_consume_binary) don't need to maintain their
 * own references to any schemas, since they'll be encapsulated in the
 * consumer that they pass their data off to.
 */

typedef struct aingle_consumer_t aingle_consumer_t;

struct aingle_consumer_t {
	/**
	 * The schema of the data that this consumer expects to process.
	 */

	aingle_schema_t  schema;

	/**
	 * Called when this consumer is freed.  This function should
	 * free any additional resources acquired by a consumer
	 * subclass.
	 */

	void (*free)(aingle_consumer_t *consumer);

	/* PRIMITIVE VALUES */

	/**
	 * Called when a boolean value is encountered.
	 */

	int (*boolean_value)(aingle_consumer_t *consumer,
			     int value,
			     void *user_data);

	/**
	 * Called when a bytes value is encountered. The @ref value
	 * pointer is only guaranteed to be valid for the duration of
	 * the callback function.  If you need to save the data for
	 * processing later, you must copy it into another buffer.
	 */

	int (*bytes_value)(aingle_consumer_t *consumer,
			   const void *value, size_t value_len,
			   void *user_data);

	/**
	 * Called when a double value is encountered.
	 */

	int (*double_value)(aingle_consumer_t *consumer,
			    double value,
			    void *user_data);

	/**
	 * Called when a float value is encountered.
	 */

	int (*float_value)(aingle_consumer_t *consumer,
			   float value,
			   void *user_data);

	/**
	 * Called when an int value is encountered.
	 */

	int (*int_value)(aingle_consumer_t *consumer,
			 int32_t value,
			 void *user_data);

	/**
	 * Called when a long value is encountered.
	 */

	int (*long_value)(aingle_consumer_t *consumer,
			  int64_t value,
			  void *user_data);

	/**
	 * Called when a null value is encountered.
	 */

	int (*null_value)(aingle_consumer_t *consumer, void *user_data);

	/**
	 * Called when a string value is encountered.  The @ref value
	 * pointer will point at UTF-8 encoded data.  (If the data
	 * you're representing isn't a UTF-8 Unicode string, you
	 * should use the bytes type.)	The @ref value_len parameter
	 * gives the length of the data in bytes, not in Unicode
	 * characters.	The @ref value pointer is only guaranteed to
	 * be valid for the duration of the callback function.	If you
	 * need to save the data for processing later, you must copy
	 * it into another buffer.
	 */

	int (*string_value)(aingle_consumer_t *consumer,
			    const void *value, size_t value_len,
			    void *user_data);

	/* COMPOUND VALUES */

	/**
	 * Called when the beginning of an array block is encountered.
	 * The @ref block_count parameter will contain the number of
	 * elements in this block.
	 */

	int (*array_start_block)(aingle_consumer_t *consumer,
				 int is_first_block,
				 unsigned int block_count,
				 void *user_data);

	/**
	 * Called before each individual element of an array is
	 * processed.  The index of the current element is passed into
	 * the callback.  The callback should fill in @ref
	 * element_consumer and @ref element_user_data with the consumer
	 * and <code>user_data</code> pointer to use to process the
	 * element.
	 */

	int (*array_element)(aingle_consumer_t *consumer,
			     unsigned int index,
			     aingle_consumer_t **element_consumer,
			     void **element_user_data,
			     void *user_data);

	/**
	 * Called when an enum value is encountered.
	 */

	int (*enum_value)(aingle_consumer_t *consumer, int value,
			  void *user_data);

	/**
	 * Called when a fixed value is encountered.  The @ref value
	 * pointer is only guaranteed to be valid for the duration of
	 * the callback function.  If you need to save the data for
	 * processing later, you must copy it into another buffer.
	 */

	int (*fixed_value)(aingle_consumer_t *consumer,
			   const void *value, size_t value_len,
			   void *user_data);

	/**
	 * Called when the beginning of a map block is encountered.
	 * The @ref block_count parameter will contain the number of
	 * elements in this block.
	 */

	int (*map_start_block)(aingle_consumer_t *consumer,
			       int is_first_block,
			       unsigned int block_count,
			       void *user_data);

	/**
	 * Called before each individual element of a map is
	 * processed.  The index and key of the current element is
	 * passed into the callback.  The key is only guaranteed to be
	 * valid for the duration of the map_element_start callback,
	 * and the map's subschema callback.  If you need to save it for
	 * later use, you must copy the key into another memory
	 * location.  The callback should fill in @ref value_consumer
	 * and @ref value_user_data with the consumer and
	 * <code>user_data</code> pointer to use to process the value.
	 */

	int (*map_element)(aingle_consumer_t *consumer,
			   unsigned int index,
			   const char *key,
			   aingle_consumer_t **value_consumer,
			   void **value_user_data,
			   void *user_data);

	/**
	 * Called when the beginning of a record is encountered.
	 */

	int (*record_start)(aingle_consumer_t *consumer,
			    void *user_data);

	/**
	 * Called before each individual field of a record is
	 * processed.  The index and name of the current field is
	 * passed into the callback.  The name is only guaranteed to
	 * be valid for the duration of the record_field_start
	 * callback, and the field's subschema callback.  If you need to
	 * save it for later use, you must copy the key into another
	 * memory location.  The callback should fill in @ref
	 * field_consumer and @ref field_user_data with the consumer
	 * <code>user_data</code> pointer to use to process the field.
	 */

	int (*record_field)(aingle_consumer_t *consumer,
			    unsigned int index,
			    aingle_consumer_t **field_consumer,
			    void **field_user_data,
			    void *user_data);

	/**
	 * Called when a union value is encountered.  The callback
	 * should fill in @ref branch_consumer and @ref branch_user_data
	 * with the consumer <code>user_data</code> pointer to use to
	 * process the branch.
	 */

	int (*union_branch)(aingle_consumer_t *consumer,
			    unsigned int discriminant,
			    aingle_consumer_t **branch_consumer,
			    void **branch_user_data,
			    void *user_data);
};


/**
 * Calls the given callback in consumer, if it's present.  If the
 * callback is NULL, it just returns a success code.
 */

#define aingle_consumer_call(consumer, callback, ...)	\
	(((consumer)->callback == NULL)? 0:		\
	 (consumer)->callback((consumer), __VA_ARGS__))


/**
 * Frees an @ref aingle_consumer_t instance.  (This function works on
 * consumer subclasses, too.)
 */

void aingle_consumer_free(aingle_consumer_t *consumer);


/*---------------------------------------------------------------------
 * Resolvers
 */

/**
 * A <i>resolver</i> is a special kind of consumer that knows how to
 * implement AIngle's schema resolution rules to translate between a
 * writer schema and a reader schema.  The consumer callbacks line up
 * with the writer schema; as each element of the datum is produced, the
 * resolver fills in the contents of an @ref aingle_datum_t instance.
 * (The datum is provided as the user_data when you use the consumer.)
 */

aingle_consumer_t *
aingle_resolver_new(aingle_schema_t writer_schema,
		  aingle_schema_t reader_schema);


/*---------------------------------------------------------------------
 * Binary encoding
 */

/**
 * Reads an AIngle datum from the given @ref aingle_reader_t.  As the
 * datum is read, each portion of it is passed off to the appropriate
 * callback in @ref consumer.
 */

int
aingle_consume_binary(aingle_reader_t reader,
		    aingle_consumer_t *consumer,
		    void *ud);


CLOSE_EXTERN
#endif
