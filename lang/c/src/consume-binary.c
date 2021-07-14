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

#include "aingle_private.h"
#include "aingle/allocation.h"
#include "aingle/consumer.h"
#include "aingle/errors.h"
#include "aingle/resolver.h"
#include "aingle/value.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "encoding.h"
#include "schema.h"
#include "datum.h"


static int
read_enum(aingle_reader_t reader, const aingle_encoding_t * enc,
	  aingle_consumer_t *consumer, void *ud)
{
	int rval;
	int64_t index;

	check_prefix(rval, enc->read_long(reader, &index),
		     "Cannot read enum value: ");
	return aingle_consumer_call(consumer, enum_value, index, ud);
}

static int
read_array(aingle_reader_t reader, const aingle_encoding_t * enc,
	   aingle_consumer_t *consumer, void *ud)
{
	int rval;
	int64_t i;          /* index within the current block */
	int64_t index = 0;  /* index within the entire array */
	int64_t block_count;
	int64_t block_size;

	check_prefix(rval, enc->read_long(reader, &block_count),
		     "Cannot read array block count: ");
	check(rval, aingle_consumer_call(consumer, array_start_block,
				       1, block_count, ud));

	while (block_count != 0) {
		if (block_count < 0) {
			block_count = block_count * -1;
			check_prefix(rval, enc->read_long(reader, &block_size),
				     "Cannot read array block size: ");
		}

		for (i = 0; i < block_count; i++, index++) {
			aingle_consumer_t  *element_consumer = NULL;
			void  *element_ud = NULL;

			check(rval,
			      aingle_consumer_call(consumer, array_element,
					         index, &element_consumer, &element_ud,
						 ud));

			check(rval, aingle_consume_binary(reader, element_consumer, element_ud));
		}

		check_prefix(rval, enc->read_long(reader, &block_count),
			     "Cannot read array block count: ");
		check(rval, aingle_consumer_call(consumer, array_start_block,
					       0, block_count, ud));
	}

	return 0;
}

static int
read_map(aingle_reader_t reader, const aingle_encoding_t * enc,
	 aingle_consumer_t *consumer, void *ud)
{
	int rval;
	int64_t i;          /* index within the current block */
	int64_t index = 0;  /* index within the entire array */
	int64_t block_count;
	int64_t block_size;

	check_prefix(rval, enc->read_long(reader, &block_count),
		     "Cannot read map block count: ");
	check(rval, aingle_consumer_call(consumer, map_start_block,
				       1, block_count, ud));

	while (block_count != 0) {
		if (block_count < 0) {
			block_count = block_count * -1;
			check_prefix(rval, enc->read_long(reader, &block_size),
				     "Cannot read map block size: ");
		}

		for (i = 0; i < block_count; i++, index++) {
			char *key;
			int64_t key_size;
			aingle_consumer_t  *element_consumer = NULL;
			void  *element_ud = NULL;

			check_prefix(rval, enc->read_string(reader, &key, &key_size),
				     "Cannot read map key: ");

			rval = aingle_consumer_call(consumer, map_element,
						  index, key,
						  &element_consumer, &element_ud,
						  ud);
			if (rval) {
				aingle_free(key, key_size);
				return rval;
			}

			rval = aingle_consume_binary(reader, element_consumer, element_ud);
			if (rval) {
				aingle_free(key, key_size);
				return rval;
			}

			aingle_free(key, key_size);
		}

		check_prefix(rval, enc->read_long(reader, &block_count),
			     "Cannot read map block count: ");
		check(rval, aingle_consumer_call(consumer, map_start_block,
					       0, block_count, ud));
	}

	return 0;
}

static int
read_union(aingle_reader_t reader, const aingle_encoding_t * enc,
	   aingle_consumer_t *consumer, void *ud)
{
	int rval;
	int64_t discriminant;
	aingle_consumer_t  *branch_consumer = NULL;
	void  *branch_ud = NULL;

	check_prefix(rval, enc->read_long(reader, &discriminant),
		     "Cannot read union discriminant: ");
	check(rval, aingle_consumer_call(consumer, union_branch,
				       discriminant,
				       &branch_consumer, &branch_ud, ud));
	return aingle_consume_binary(reader, branch_consumer, branch_ud);
}

static int
read_record(aingle_reader_t reader, const aingle_encoding_t * enc,
	    aingle_consumer_t *consumer, void *ud)
{
	int rval;
	size_t  num_fields;
	unsigned int  i;

	AINGLE_UNUSED(enc);

	check(rval, aingle_consumer_call(consumer, record_start, ud));

	num_fields = aingle_schema_record_size(consumer->schema);
	for (i = 0; i < num_fields; i++) {
		aingle_consumer_t  *field_consumer = NULL;
		void  *field_ud = NULL;

		check(rval, aingle_consumer_call(consumer, record_field,
					       i, &field_consumer, &field_ud,
					       ud));

		if (field_consumer) {
			check(rval, aingle_consume_binary(reader, field_consumer, field_ud));
		} else {
			aingle_schema_t  field_schema =
			    aingle_schema_record_field_get_by_index(consumer->schema, i);
			check(rval, aingle_skip_data(reader, field_schema));
		}
	}

	return 0;
}

int
aingle_consume_binary(aingle_reader_t reader, aingle_consumer_t *consumer, void *ud)
{
	int rval;
	const aingle_encoding_t *enc = &aingle_binary_encoding;

	check_param(EINVAL, reader, "reader");
	check_param(EINVAL, consumer, "consumer");

	switch (aingle_typeof(consumer->schema)) {
	case AINGLE_NULL:
		check_prefix(rval, enc->read_null(reader),
			     "Cannot read null value: ");
		check(rval, aingle_consumer_call(consumer, null_value, ud));
		break;

	case AINGLE_BOOLEAN:
		{
			int8_t b;
			check_prefix(rval, enc->read_boolean(reader, &b),
				     "Cannot read boolean value: ");
			check(rval, aingle_consumer_call(consumer, boolean_value, b, ud));
		}
		break;

	case AINGLE_STRING:
		{
			int64_t len;
			char *s;
			check_prefix(rval, enc->read_string(reader, &s, &len),
				     "Cannot read string value: ");
			check(rval, aingle_consumer_call(consumer, string_value, s, len, ud));
		}
		break;

	case AINGLE_INT32:
		{
			int32_t i;
			check_prefix(rval, enc->read_int(reader, &i),
				    "Cannot read int value: ");
			check(rval, aingle_consumer_call(consumer, int_value, i, ud));
		}
		break;

	case AINGLE_INT64:
		{
			int64_t l;
			check_prefix(rval, enc->read_long(reader, &l),
				     "Cannot read long value: ");
			check(rval, aingle_consumer_call(consumer, long_value, l, ud));
		}
		break;

	case AINGLE_FLOAT:
		{
			float f;
			check_prefix(rval, enc->read_float(reader, &f),
				     "Cannot read float value: ");
			check(rval, aingle_consumer_call(consumer, float_value, f, ud));
		}
		break;

	case AINGLE_DOUBLE:
		{
			double d;
			check_prefix(rval, enc->read_double(reader, &d),
				     "Cannot read double value: ");
			check(rval, aingle_consumer_call(consumer, double_value, d, ud));
		}
		break;

	case AINGLE_BYTES:
		{
			char *bytes;
			int64_t len;
			check_prefix(rval, enc->read_bytes(reader, &bytes, &len),
				     "Cannot read bytes value: ");
			check(rval, aingle_consumer_call(consumer, bytes_value, bytes, len, ud));
		}
		break;

	case AINGLE_FIXED:
		{
			char *bytes;
			int64_t size =
			    aingle_schema_to_fixed(consumer->schema)->size;

			bytes = (char *) aingle_malloc(size);
			if (!bytes) {
				aingle_prefix_error("Cannot allocate new fixed value");
				return ENOMEM;
			}
			rval = aingle_read(reader, bytes, size);
			if (rval) {
				aingle_prefix_error("Cannot read fixed value: ");
				aingle_free(bytes, size);
				return rval;
			}

			rval = aingle_consumer_call(consumer, fixed_value, bytes, size, ud);
			if (rval) {
				aingle_free(bytes, size);
				return rval;
			}
		}
		break;

	case AINGLE_ENUM:
		check(rval, read_enum(reader, enc, consumer, ud));
		break;

	case AINGLE_ARRAY:
		check(rval, read_array(reader, enc, consumer, ud));
		break;

	case AINGLE_MAP:
		check(rval, read_map(reader, enc, consumer, ud));
		break;

	case AINGLE_UNION:
		check(rval, read_union(reader, enc, consumer, ud));
		break;

	case AINGLE_RECORD:
		check(rval, read_record(reader, enc, consumer, ud));
		break;

	case AINGLE_LINK:
		aingle_set_error("Consumer can't consume a link schema directly");
		return EINVAL;
	}

	return 0;
}
