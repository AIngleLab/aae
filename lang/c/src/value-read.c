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

#include <aingle/platform.h>
#include <stdlib.h>
#include <string.h>

#include "aingle/allocation.h"
#include "aingle/basics.h"
#include "aingle/data.h"
#include "aingle/io.h"
#include "aingle/value.h"
#include "aingle_private.h"
#include "encoding.h"


/*
 * Forward declaration; this is basically the same as aingle_value_read,
 * but it doesn't reset dest first.  (Since it will have already been
 * reset in aingle_value_read itself).
 */

static int
read_value(aingle_reader_t reader, aingle_value_t *dest);


static int
read_array_value(aingle_reader_t reader, aingle_value_t *dest)
{
	int  rval;
	size_t  i;          /* index within the current block */
	size_t  index = 0;  /* index within the entire array */
	int64_t  block_count;
	int64_t  block_size;

	check_prefix(rval, aingle_binary_encoding.
		     read_long(reader, &block_count),
		     "Cannot read array block count: ");

	while (block_count != 0) {
		if (block_count < 0) {
			block_count = block_count * -1;
			check_prefix(rval, aingle_binary_encoding.
				     read_long(reader, &block_size),
				     "Cannot read array block size: ");
		}

		for (i = 0; i < (size_t) block_count; i++, index++) {
			aingle_value_t  child;

			check(rval, aingle_value_append(dest, &child, NULL));
			check(rval, read_value(reader, &child));
		}

		check_prefix(rval, aingle_binary_encoding.
			     read_long(reader, &block_count),
			     "Cannot read array block count: ");
	}

	return 0;
}


static int
read_map_value(aingle_reader_t reader, aingle_value_t *dest)
{
	int  rval;
	size_t  i;          /* index within the current block */
	size_t  index = 0;  /* index within the entire array */
	int64_t  block_count;
	int64_t  block_size;

	check_prefix(rval, aingle_binary_encoding.read_long(reader, &block_count),
		     "Cannot read map block count: ");

	while (block_count != 0) {
		if (block_count < 0) {
			block_count = block_count * -1;
			check_prefix(rval, aingle_binary_encoding.
				     read_long(reader, &block_size),
				     "Cannot read map block size: ");
		}

		for (i = 0; i < (size_t) block_count; i++, index++) {
			char *key;
			int64_t key_size;
			aingle_value_t  child;

			check_prefix(rval, aingle_binary_encoding.
				     read_string(reader, &key, &key_size),
				     "Cannot read map key: ");

			rval = aingle_value_add(dest, key, &child, NULL, NULL);
			if (rval) {
				aingle_free(key, key_size);
				return rval;
			}

			rval = read_value(reader, &child);
			if (rval) {
				aingle_free(key, key_size);
				return rval;
			}

			aingle_free(key, key_size);
		}

		check_prefix(rval, aingle_binary_encoding.
			     read_long(reader, &block_count),
			     "Cannot read map block count: ");
	}

	return 0;
}


static int
read_record_value(aingle_reader_t reader, aingle_value_t *dest)
{
	int  rval;
	size_t  field_count;
	size_t  i;

	aingle_schema_t  record_schema = aingle_value_get_schema(dest);

	check(rval, aingle_value_get_size(dest, &field_count));
	for (i = 0; i < field_count; i++) {
		aingle_value_t  field;

		check(rval, aingle_value_get_by_index(dest, i, &field, NULL));
		if (field.iface != NULL) {
			check(rval, read_value(reader, &field));
		} else {
			aingle_schema_t  field_schema =
			    aingle_schema_record_field_get_by_index(record_schema, i);
			check(rval, aingle_skip_data(reader, field_schema));
		}
	}

	return 0;
}


static int
read_union_value(aingle_reader_t reader, aingle_value_t *dest)
{
	int rval;
	int64_t discriminant;
	aingle_schema_t  union_schema;
	int64_t  branch_count;
	aingle_value_t  branch;

	check_prefix(rval, aingle_binary_encoding.
		     read_long(reader, &discriminant),
		     "Cannot read union discriminant: ");

	union_schema = aingle_value_get_schema(dest);
	branch_count = aingle_schema_union_size(union_schema);

	if (discriminant < 0 || discriminant >= branch_count) {
		aingle_set_error("Invalid union discriminant value: (%d)",
			       discriminant);
		return 1;
	}

	check(rval, aingle_value_set_branch(dest, discriminant, &branch));
	check(rval, read_value(reader, &branch));
	return 0;
}


/*
 * A wrapped buffer implementation that takes control of a buffer
 * allocated using aingle_malloc.
 */

struct aingle_wrapped_alloc {
	const void  *original;
	size_t  allocated_size;
};

static void
aingle_wrapped_alloc_free(aingle_wrapped_buffer_t *self)
{
	struct aingle_wrapped_alloc  *alloc = (struct aingle_wrapped_alloc *) self->user_data;
	aingle_free((void *) alloc->original, alloc->allocated_size);
	aingle_freet(struct aingle_wrapped_alloc, alloc);
}

static int
aingle_wrapped_alloc_new(aingle_wrapped_buffer_t *dest,
		       const void *buf, size_t length)
{
	struct aingle_wrapped_alloc  *alloc = (struct aingle_wrapped_alloc *) aingle_new(struct aingle_wrapped_alloc);
	if (alloc == NULL) {
		return ENOMEM;
	}

	dest->buf = buf;
	dest->size = length;
	dest->user_data = alloc;
	dest->free = aingle_wrapped_alloc_free;
	dest->copy = NULL;
	dest->slice = NULL;

	alloc->original = buf;
	alloc->allocated_size = length;
	return 0;
}


static int
read_value(aingle_reader_t reader, aingle_value_t *dest)
{
	int  rval;

	switch (aingle_value_get_type(dest)) {
		case AINGLE_BOOLEAN:
		{
			int8_t  val;
			check_prefix(rval, aingle_binary_encoding.
				     read_boolean(reader, &val),
				     "Cannot read boolean value: ");
			return aingle_value_set_boolean(dest, val);
		}

		case AINGLE_BYTES:
		{
			char  *bytes;
			int64_t  len;
			check_prefix(rval, aingle_binary_encoding.
				     read_bytes(reader, &bytes, &len),
				     "Cannot read bytes value: ");

			/*
			 * read_bytes allocates an extra byte to always
			 * ensure that the data is NUL terminated, but
			 * that byte isn't included in the length.  We
			 * include that extra byte in the allocated
			 * size, but not in the length of the buffer.
			 */

			aingle_wrapped_buffer_t  buf;
			check(rval, aingle_wrapped_alloc_new(&buf, bytes, len+1));
			buf.size--;
			return aingle_value_give_bytes(dest, &buf);
		}

		case AINGLE_DOUBLE:
		{
			double  val;
			check_prefix(rval, aingle_binary_encoding.
				     read_double(reader, &val),
				     "Cannot read double value: ");
			return aingle_value_set_double(dest, val);
		}

		case AINGLE_FLOAT:
		{
			float  val;
			check_prefix(rval, aingle_binary_encoding.
				     read_float(reader, &val),
				     "Cannot read float value: ");
			return aingle_value_set_float(dest, val);
		}

		case AINGLE_INT32:
		{
			int32_t  val;
			check_prefix(rval, aingle_binary_encoding.
				     read_int(reader, &val),
				     "Cannot read int value: ");
			return aingle_value_set_int(dest, val);
		}

		case AINGLE_INT64:
		{
			int64_t  val;
			check_prefix(rval, aingle_binary_encoding.
				     read_long(reader, &val),
				     "Cannot read long value: ");
			return aingle_value_set_long(dest, val);
		}

		case AINGLE_NULL:
		{
			check_prefix(rval, aingle_binary_encoding.
				     read_null(reader),
				     "Cannot read null value: ");
			return aingle_value_set_null(dest);
		}

		case AINGLE_STRING:
		{
			char  *str;
			int64_t  size;

			/*
			 * read_string returns a size that includes the
			 * NUL terminator, and the free function will be
			 * called with a size that also includes the NUL
			 */

			check_prefix(rval, aingle_binary_encoding.
				     read_string(reader, &str, &size),
				     "Cannot read string value: ");

			aingle_wrapped_buffer_t  buf;
			check(rval, aingle_wrapped_alloc_new(&buf, str, size));
			return aingle_value_give_string_len(dest, &buf);
		}

		case AINGLE_ARRAY:
			return read_array_value(reader, dest);

		case AINGLE_ENUM:
		{
			int64_t  val;
			check_prefix(rval, aingle_binary_encoding.
				     read_long(reader, &val),
				     "Cannot read enum value: ");
			return aingle_value_set_enum(dest, val);
		}

		case AINGLE_FIXED:
		{
			aingle_schema_t  schema = aingle_value_get_schema(dest);
			char *bytes;
			int64_t size = aingle_schema_fixed_size(schema);

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

			aingle_wrapped_buffer_t  buf;
			rval = aingle_wrapped_alloc_new(&buf, bytes, size);
			if (rval != 0) {
				aingle_free(bytes, size);
				return rval;
			}

			return aingle_value_give_fixed(dest, &buf);
		}

		case AINGLE_MAP:
			return read_map_value(reader, dest);

		case AINGLE_RECORD:
			return read_record_value(reader, dest);

		case AINGLE_UNION:
			return read_union_value(reader, dest);

		default:
		{
			aingle_set_error("Unknown schema type");
			return EINVAL;
		}
	}

	return 0;
}

int
aingle_value_read(aingle_reader_t reader, aingle_value_t *dest)
{
	int  rval;
	check(rval, aingle_value_reset(dest));
	return read_value(reader, dest);
}
