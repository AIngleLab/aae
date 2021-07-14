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

#include "aingle/basics.h"
#include "aingle/io.h"
#include "aingle/value.h"
#include "aingle_private.h"
#include "encoding.h"


static int
write_array_value(aingle_writer_t writer, aingle_value_t *src)
{
	int  rval;
	size_t  element_count;
	check(rval, aingle_value_get_size(src, &element_count));

	if (element_count > 0) {
		check_prefix(rval, aingle_binary_encoding.write_long
			     (writer, element_count),
			     "Cannot write array block count: ");

		size_t  i;
		for (i = 0; i < element_count; i++) {
			aingle_value_t  child;
			check(rval, aingle_value_get_by_index(src, i, &child, NULL));
			check(rval, aingle_value_write(writer, &child));
		}
	}

	check_prefix(rval, aingle_binary_encoding.write_long(writer, 0),
		     "Cannot write array block count: ");
	return 0;
}


static int
write_map_value(aingle_writer_t writer, aingle_value_t *src)
{
	int  rval;
	size_t  element_count;
	check(rval, aingle_value_get_size(src, &element_count));

	if (element_count > 0) {
		check_prefix(rval, aingle_binary_encoding.write_long
			     (writer, element_count),
			     "Cannot write map block count: ");

		size_t  i;
		for (i = 0; i < element_count; i++) {
			aingle_value_t  child;
			const char  *key;
			check(rval, aingle_value_get_by_index(src, i, &child, &key));
			check(rval, aingle_binary_encoding.write_string(writer, key));
			check(rval, aingle_value_write(writer, &child));
		}
	}

	check_prefix(rval, aingle_binary_encoding.write_long(writer, 0),
		     "Cannot write map block count: ");
	return 0;
}

static int
write_record_value(aingle_writer_t writer, aingle_value_t *src)
{
	int  rval;
	size_t  field_count;
	check(rval, aingle_value_get_size(src, &field_count));

	size_t  i;
	for (i = 0; i < field_count; i++) {
		aingle_value_t  field;
		check(rval, aingle_value_get_by_index(src, i, &field, NULL));
		check(rval, aingle_value_write(writer, &field));
	}

	return 0;
}

static int
write_union_value(aingle_writer_t writer, aingle_value_t *src)
{
	int  rval;
	int  discriminant;
	aingle_value_t  branch;

	check(rval, aingle_value_get_discriminant(src, &discriminant));
	check(rval, aingle_value_get_current_branch(src, &branch));
	check(rval, aingle_binary_encoding.write_long(writer, discriminant));
	return aingle_value_write(writer, &branch);
}

int
aingle_value_write(aingle_writer_t writer, aingle_value_t *src)
{
	int  rval;

	switch (aingle_value_get_type(src)) {
		case AINGLE_BOOLEAN:
		{
			int  val;
			check(rval, aingle_value_get_boolean(src, &val));
			return aingle_binary_encoding.write_boolean(writer, val);
		}

		case AINGLE_BYTES:
		{
			const void  *buf;
			size_t  size;
			check(rval, aingle_value_get_bytes(src, &buf, &size));
			return aingle_binary_encoding.write_bytes(writer, (const char *) buf, size);
		}

		case AINGLE_DOUBLE:
		{
			double  val;
			check(rval, aingle_value_get_double(src, &val));
			return aingle_binary_encoding.write_double(writer, val);
		}

		case AINGLE_FLOAT:
		{
			float  val;
			check(rval, aingle_value_get_float(src, &val));
			return aingle_binary_encoding.write_float(writer, val);
		}

		case AINGLE_INT32:
		{
			int32_t  val;
			check(rval, aingle_value_get_int(src, &val));
			return aingle_binary_encoding.write_long(writer, val);
		}

		case AINGLE_INT64:
		{
			int64_t  val;
			check(rval, aingle_value_get_long(src, &val));
			return aingle_binary_encoding.write_long(writer, val);
		}

		case AINGLE_NULL:
		{
			check(rval, aingle_value_get_null(src));
			return aingle_binary_encoding.write_null(writer);
		}

		case AINGLE_STRING:
		{
			const char  *str;
			size_t  size;
			check(rval, aingle_value_get_string(src, &str, &size));
			return aingle_binary_encoding.write_bytes(writer, str, size-1);
		}

		case AINGLE_ARRAY:
			return write_array_value(writer, src);

		case AINGLE_ENUM:
		{
			int  val;
			check(rval, aingle_value_get_enum(src, &val));
			return aingle_binary_encoding.write_long(writer, val);
		}

		case AINGLE_FIXED:
		{
			const void  *buf;
			size_t  size;
			check(rval, aingle_value_get_fixed(src, &buf, &size));
			return aingle_write(writer, (void *) buf, size);
		}

		case AINGLE_MAP:
			return write_map_value(writer, src);

		case AINGLE_RECORD:
			return write_record_value(writer, src);

		case AINGLE_UNION:
			return write_union_value(writer, src);

		default:
		{
			aingle_set_error("Unknown schema type");
			return EINVAL;
		}
	}

	return 0;
}
