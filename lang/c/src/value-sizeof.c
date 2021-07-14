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


/*
 * Forward declaration; this is basically the same as aingle_value_sizeof,
 * but it doesn't initialize size first.  (Since it will have already
 * been initialized in aingle_value_sizeof itself).
 */

static int
sizeof_value(aingle_value_t *src, size_t *size);


static int
sizeof_array_value(aingle_value_t *src, size_t *size)
{
	int  rval;
	size_t  element_count;
	check(rval, aingle_value_get_size(src, &element_count));

	if (element_count > 0) {
		*size += aingle_binary_encoding.size_long(NULL, element_count);

		size_t  i;
		for (i = 0; i < element_count; i++) {
			aingle_value_t  child;
			check(rval, aingle_value_get_by_index(src, i, &child, NULL));
			check(rval, sizeof_value(&child, size));
		}
	}

	*size += aingle_binary_encoding.size_long(NULL, 0);
	return 0;
}


static int
sizeof_map_value(aingle_value_t *src, size_t *size)
{
	int  rval;
	size_t  element_count;
	check(rval, aingle_value_get_size(src, &element_count));

	if (element_count > 0) {
		*size += aingle_binary_encoding.size_long(NULL, element_count);

		size_t  i;
		for (i = 0; i < element_count; i++) {
			aingle_value_t  child;
			const char  *key;
			check(rval, aingle_value_get_by_index(src, i, &child, &key));
			*size += aingle_binary_encoding.size_string(NULL, key);
			check(rval, sizeof_value(&child, size));
		}
	}

	*size += aingle_binary_encoding.size_long(NULL, 0);
	return 0;
}

static int
sizeof_record_value(aingle_value_t *src, size_t *size)
{
	int  rval;
	size_t  field_count;
	check(rval, aingle_value_get_size(src, &field_count));

	size_t  i;
	for (i = 0; i < field_count; i++) {
		aingle_value_t  field;
		check(rval, aingle_value_get_by_index(src, i, &field, NULL));
		check(rval, sizeof_value(&field, size));
	}

	return 0;
}

static int
sizeof_union_value(aingle_value_t *src, size_t *size)
{
	int  rval;
	int  discriminant;
	aingle_value_t  branch;

	check(rval, aingle_value_get_discriminant(src, &discriminant));
	check(rval, aingle_value_get_current_branch(src, &branch));
	*size += aingle_binary_encoding.size_long(NULL, discriminant);
	return sizeof_value(&branch, size);
}

static int
sizeof_value(aingle_value_t *src, size_t *size)
{
	int  rval;

	switch (aingle_value_get_type(src)) {
		case AINGLE_BOOLEAN:
		{
			int  val;
			check(rval, aingle_value_get_boolean(src, &val));
			*size += aingle_binary_encoding.size_boolean(NULL, val);
			return 0;
		}

		case AINGLE_BYTES:
		{
			const void  *buf;
			size_t  sz;
			check(rval, aingle_value_get_bytes(src, &buf, &sz));
			*size += aingle_binary_encoding.size_bytes(NULL, (const char *) buf, sz);
			return 0;
		}

		case AINGLE_DOUBLE:
		{
			double  val;
			check(rval, aingle_value_get_double(src, &val));
			*size += aingle_binary_encoding.size_double(NULL, val);
			return 0;
		}

		case AINGLE_FLOAT:
		{
			float  val;
			check(rval, aingle_value_get_float(src, &val));
			*size += aingle_binary_encoding.size_float(NULL, val);
			return 0;
		}

		case AINGLE_INT32:
		{
			int32_t  val;
			check(rval, aingle_value_get_int(src, &val));
			*size += aingle_binary_encoding.size_long(NULL, val);
			return 0;
		}

		case AINGLE_INT64:
		{
			int64_t  val;
			check(rval, aingle_value_get_long(src, &val));
			*size += aingle_binary_encoding.size_long(NULL, val);
			return 0;
		}

		case AINGLE_NULL:
		{
			check(rval, aingle_value_get_null(src));
			*size += aingle_binary_encoding.size_null(NULL);
			return 0;
		}

		case AINGLE_STRING:
		{
			const char  *str;
			size_t  sz;
			check(rval, aingle_value_get_string(src, &str, &sz));
			*size += aingle_binary_encoding.size_bytes(NULL, str, sz-1);
			return 0;
		}

		case AINGLE_ARRAY:
			return sizeof_array_value(src, size);

		case AINGLE_ENUM:
		{
			int  val;
			check(rval, aingle_value_get_enum(src, &val));
			*size += aingle_binary_encoding.size_long(NULL, val);
			return 0;
		}

		case AINGLE_FIXED:
		{
			size_t  sz;
			check(rval, aingle_value_get_fixed(src, NULL, &sz));
			*size += sz;
			return 0;
		}

		case AINGLE_MAP:
			return sizeof_map_value(src, size);

		case AINGLE_RECORD:
			return sizeof_record_value(src, size);

		case AINGLE_UNION:
			return sizeof_union_value(src, size);

		default:
		{
			aingle_set_error("Unknown schema type");
			return EINVAL;
		}
	}

	return 0;
}

int
aingle_value_sizeof(aingle_value_t *src, size_t *size)
{
	check_param(EINVAL, size, "size pointer");
	*size = 0;
	return sizeof_value(src, size);
}
