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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "aingle/allocation.h"
#include "aingle/data.h"
#include "aingle/errors.h"
#include "aingle/value.h"
#include "aingle_private.h"


#define check_return(retval, call) \
	do { \
		int  rval = call; \
		if (rval != 0) { return (retval); } \
	} while (0)


void
aingle_value_incref(aingle_value_t *value)
{
	value->iface->incref(value);
}

void
aingle_value_decref(aingle_value_t *value)
{
	value->iface->decref(value);
	aingle_value_iface_decref(value->iface);
	value->iface = NULL;
	value->self = NULL;
}

void
aingle_value_copy_ref(aingle_value_t *dest, const aingle_value_t *src)
{
	dest->iface = src->iface;
	dest->self = src->self;
	aingle_value_iface_incref(dest->iface);
	dest->iface->incref(dest);
}

void
aingle_value_move_ref(aingle_value_t *dest, aingle_value_t *src)
{
	dest->iface = src->iface;
	dest->self = src->self;
	src->iface = NULL;
	src->self = NULL;
}


int
aingle_value_equal_fast(aingle_value_t *val1, aingle_value_t *val2)
{
	aingle_type_t  type1 = aingle_value_get_type(val1);
	aingle_type_t  type2 = aingle_value_get_type(val2);
	if (type1 != type2) {
		return 0;
	}

	switch (type1) {
		case AINGLE_BOOLEAN:
		{
			int  v1;
			int  v2;
			check_return(0, aingle_value_get_boolean(val1, &v1));
			check_return(0, aingle_value_get_boolean(val2, &v2));
			return (v1 == v2);
		}

		case AINGLE_BYTES:
		{
			const void  *buf1;
			const void  *buf2;
			size_t  size1;
			size_t  size2;
			check_return(0, aingle_value_get_bytes(val1, &buf1, &size1));
			check_return(0, aingle_value_get_bytes(val2, &buf2, &size2));
			if (size1 != size2) {
				return 0;
			}
			return (memcmp(buf1, buf2, size1) == 0);
		}

		case AINGLE_DOUBLE:
		{
			double  v1;
			double  v2;
			check_return(0, aingle_value_get_double(val1, &v1));
			check_return(0, aingle_value_get_double(val2, &v2));
			return (v1 == v2);
		}

		case AINGLE_FLOAT:
		{
			float  v1;
			float  v2;
			check_return(0, aingle_value_get_float(val1, &v1));
			check_return(0, aingle_value_get_float(val2, &v2));
			return (v1 == v2);
		}

		case AINGLE_INT32:
		{
			int32_t  v1;
			int32_t  v2;
			check_return(0, aingle_value_get_int(val1, &v1));
			check_return(0, aingle_value_get_int(val2, &v2));
			return (v1 == v2);
		}

		case AINGLE_INT64:
		{
			int64_t  v1;
			int64_t  v2;
			check_return(0, aingle_value_get_long(val1, &v1));
			check_return(0, aingle_value_get_long(val2, &v2));
			return (v1 == v2);
		}

		case AINGLE_NULL:
		{
			check_return(0, aingle_value_get_null(val1));
			check_return(0, aingle_value_get_null(val2));
			return 1;
		}

		case AINGLE_STRING:
		{
			const char  *buf1;
			const char  *buf2;
			size_t  size1;
			size_t  size2;
			check_return(0, aingle_value_get_string(val1, &buf1, &size1));
			check_return(0, aingle_value_get_string(val2, &buf2, &size2));
			if (size1 != size2) {
				return 0;
			}
			return (memcmp(buf1, buf2, size1) == 0);
		}

		case AINGLE_ARRAY:
		{
			size_t  count1;
			size_t  count2;
			check_return(0, aingle_value_get_size(val1, &count1));
			check_return(0, aingle_value_get_size(val2, &count2));
			if (count1 != count2) {
				return 0;
			}

			size_t  i;
			for (i = 0; i < count1; i++) {
				aingle_value_t  child1;
				aingle_value_t  child2;
				check_return(0, aingle_value_get_by_index
					     (val1, i, &child1, NULL));
				check_return(0, aingle_value_get_by_index
					     (val2, i, &child2, NULL));
				if (!aingle_value_equal_fast(&child1, &child2)) {
					return 0;
				}
			}

			return 1;
		}

		case AINGLE_ENUM:
		{
			int  v1;
			int  v2;
			check_return(0, aingle_value_get_enum(val1, &v1));
			check_return(0, aingle_value_get_enum(val2, &v2));
			return (v1 == v2);
		}

		case AINGLE_FIXED:
		{
			const void  *buf1;
			const void  *buf2;
			size_t  size1;
			size_t  size2;
			check_return(0, aingle_value_get_fixed(val1, &buf1, &size1));
			check_return(0, aingle_value_get_fixed(val2, &buf2, &size2));
			if (size1 != size2) {
				return 0;
			}
			return (memcmp(buf1, buf2, size1) == 0);
		}

		case AINGLE_MAP:
		{
			size_t  count1;
			size_t  count2;
			check_return(0, aingle_value_get_size(val1, &count1));
			check_return(0, aingle_value_get_size(val2, &count2));
			if (count1 != count2) {
				return 0;
			}

			size_t  i;
			for (i = 0; i < count1; i++) {
				aingle_value_t  child1;
				aingle_value_t  child2;
				const char  *key1;
				check_return(0, aingle_value_get_by_index
					     (val1, i, &child1, &key1));
				check_return(0, aingle_value_get_by_name
					     (val2, key1, &child2, NULL));
				if (!aingle_value_equal_fast(&child1, &child2)) {
					return 0;
				}
			}

			return 1;
		}

		case AINGLE_RECORD:
		{
			size_t  count1;
			check_return(0, aingle_value_get_size(val1, &count1));

			size_t  i;
			for (i = 0; i < count1; i++) {
				aingle_value_t  child1;
				aingle_value_t  child2;
				check_return(0, aingle_value_get_by_index
					     (val1, i, &child1, NULL));
				check_return(0, aingle_value_get_by_index
					     (val2, i, &child2, NULL));
				if (!aingle_value_equal_fast(&child1, &child2)) {
					return 0;
				}
			}

			return 1;
		}

		case AINGLE_UNION:
		{
			int  disc1;
			int  disc2;
			check_return(0, aingle_value_get_discriminant(val1, &disc1));
			check_return(0, aingle_value_get_discriminant(val2, &disc2));
			if (disc1 != disc2) {
				return 0;
			}

			aingle_value_t  branch1;
			aingle_value_t  branch2;
			check_return(0, aingle_value_get_current_branch(val1, &branch1));
			check_return(0, aingle_value_get_current_branch(val2, &branch2));
			return aingle_value_equal_fast(&branch1, &branch2);
		}

		default:
			return 0;
	}
}

int
aingle_value_equal(aingle_value_t *val1, aingle_value_t *val2)
{
	aingle_schema_t  schema1 = aingle_value_get_schema(val1);
	aingle_schema_t  schema2 = aingle_value_get_schema(val2);
	if (!aingle_schema_equal(schema1, schema2)) {
		return 0;
	}

	return aingle_value_equal_fast(val1, val2);
}


#define cmp(v1, v2) \
	(((v1) == (v2))? 0: \
	 ((v1) <  (v2))? -1: 1)
int
aingle_value_cmp_fast(aingle_value_t *val1, aingle_value_t *val2)
{
	aingle_type_t  type1 = aingle_value_get_type(val1);
	aingle_type_t  type2 = aingle_value_get_type(val2);
	if (type1 != type2) {
		return -1;
	}

	switch (type1) {
		case AINGLE_BOOLEAN:
		{
			int  v1;
			int  v2;
			check_return(0, aingle_value_get_boolean(val1, &v1));
			check_return(0, aingle_value_get_boolean(val2, &v2));
			return cmp(!!v1, !!v2);
		}

		case AINGLE_BYTES:
		{
			const void  *buf1;
			const void  *buf2;
			size_t  size1;
			size_t  size2;
			size_t  min_size;
			int  result;

			check_return(0, aingle_value_get_bytes(val1, &buf1, &size1));
			check_return(0, aingle_value_get_bytes(val2, &buf2, &size2));

			min_size = (size1 < size2)? size1: size2;
			result = memcmp(buf1, buf2, min_size);
			if (result != 0) {
				return result;
			} else {
				return cmp(size1, size2);
			}
		}

		case AINGLE_DOUBLE:
		{
			double  v1;
			double  v2;
			check_return(0, aingle_value_get_double(val1, &v1));
			check_return(0, aingle_value_get_double(val2, &v2));
			return cmp(v1, v2);
		}

		case AINGLE_FLOAT:
		{
			float  v1;
			float  v2;
			check_return(0, aingle_value_get_float(val1, &v1));
			check_return(0, aingle_value_get_float(val2, &v2));
			return cmp(v1, v2);
		}

		case AINGLE_INT32:
		{
			int32_t  v1;
			int32_t  v2;
			check_return(0, aingle_value_get_int(val1, &v1));
			check_return(0, aingle_value_get_int(val2, &v2));
			return cmp(v1, v2);
		}

		case AINGLE_INT64:
		{
			int64_t  v1;
			int64_t  v2;
			check_return(0, aingle_value_get_long(val1, &v1));
			check_return(0, aingle_value_get_long(val2, &v2));
			return cmp(v1, v2);
		}

		case AINGLE_NULL:
		{
			check_return(0, aingle_value_get_null(val1));
			check_return(0, aingle_value_get_null(val2));
			return 0;
		}

		case AINGLE_STRING:
		{
			const char  *buf1;
			const char  *buf2;
			size_t  size1;
			size_t  size2;
			size_t  min_size;
			int  result;
			check_return(0, aingle_value_get_string(val1, &buf1, &size1));
			check_return(0, aingle_value_get_string(val2, &buf2, &size2));

			min_size = (size1 < size2)? size1: size2;
			result = memcmp(buf1, buf2, min_size);
			if (result != 0) {
				return result;
			} else {
				return cmp(size1, size2);
			}
		}

		case AINGLE_ARRAY:
		{
			size_t  count1;
			size_t  count2;
			size_t  min_count;
			size_t  i;
			check_return(0, aingle_value_get_size(val1, &count1));
			check_return(0, aingle_value_get_size(val2, &count2));

			min_count = (count1 < count2)? count1: count2;
			for (i = 0; i < min_count; i++) {
				aingle_value_t  child1;
				aingle_value_t  child2;
				int  result;
				check_return(0, aingle_value_get_by_index
					     (val1, i, &child1, NULL));
				check_return(0, aingle_value_get_by_index
					     (val2, i, &child2, NULL));
				result = aingle_value_cmp_fast(&child1, &child2);
				if (result != 0) {
					return result;
				}
			}

			return cmp(count1, count2);
		}

		case AINGLE_ENUM:
		{
			int  v1;
			int  v2;
			check_return(0, aingle_value_get_enum(val1, &v1));
			check_return(0, aingle_value_get_enum(val2, &v2));
			return cmp(v1, v2);
		}

		case AINGLE_FIXED:
		{
			const void  *buf1;
			const void  *buf2;
			size_t  size1;
			size_t  size2;
			check_return(0, aingle_value_get_fixed(val1, &buf1, &size1));
			check_return(0, aingle_value_get_fixed(val2, &buf2, &size2));
			if (size1 != size2) {
				return -1;
			}
			return memcmp(buf1, buf2, size1);
		}

		case AINGLE_MAP:
		{
			return -1;
		}

		case AINGLE_RECORD:
		{
			size_t  count1;
			check_return(0, aingle_value_get_size(val1, &count1));

			size_t  i;
			for (i = 0; i < count1; i++) {
				aingle_value_t  child1;
				aingle_value_t  child2;
				int  result;

				check_return(0, aingle_value_get_by_index
					     (val1, i, &child1, NULL));
				check_return(0, aingle_value_get_by_index
					     (val2, i, &child2, NULL));
				result = aingle_value_cmp_fast(&child1, &child2);
				if (result != 0) {
					return result;
				}
			}

			return 0;
		}

		case AINGLE_UNION:
		{
			int  disc1;
			int  disc2;
			check_return(0, aingle_value_get_discriminant(val1, &disc1));
			check_return(0, aingle_value_get_discriminant(val2, &disc2));

			if (disc1 == disc2) {
				aingle_value_t  branch1;
				aingle_value_t  branch2;
				check_return(0, aingle_value_get_current_branch(val1, &branch1));
				check_return(0, aingle_value_get_current_branch(val2, &branch2));
				return aingle_value_cmp_fast(&branch1, &branch2);
			} else {
				return cmp(disc1, disc2);
			}
		}

		default:
			return 0;
	}
}

int
aingle_value_cmp(aingle_value_t *val1, aingle_value_t *val2)
{
	aingle_schema_t  schema1 = aingle_value_get_schema(val1);
	aingle_schema_t  schema2 = aingle_value_get_schema(val2);
	if (!aingle_schema_equal(schema1, schema2)) {
		return 0;
	}

	return aingle_value_cmp_fast(val1, val2);
}


int
aingle_value_copy_fast(aingle_value_t *dest, const aingle_value_t *src)
{
	aingle_type_t  dest_type = aingle_value_get_type(dest);
	aingle_type_t  src_type = aingle_value_get_type(src);
	if (dest_type != src_type) {
		return 0;
	}

	int  rval;
	check(rval, aingle_value_reset(dest));

	switch (dest_type) {
		case AINGLE_BOOLEAN:
		{
			int  val;
			check(rval, aingle_value_get_boolean(src, &val));
			return aingle_value_set_boolean(dest, val);
		}

		case AINGLE_BYTES:
		{
			aingle_wrapped_buffer_t  val;
			check(rval, aingle_value_grab_bytes(src, &val));
			return aingle_value_give_bytes(dest, &val);
		}

		case AINGLE_DOUBLE:
		{
			double  val;
			check(rval, aingle_value_get_double(src, &val));
			return aingle_value_set_double(dest, val);
		}

		case AINGLE_FLOAT:
		{
			float  val;
			check(rval, aingle_value_get_float(src, &val));
			return aingle_value_set_float(dest, val);
		}

		case AINGLE_INT32:
		{
			int32_t  val;
			check(rval, aingle_value_get_int(src, &val));
			return aingle_value_set_int(dest, val);
		}

		case AINGLE_INT64:
		{
			int64_t  val;
			check(rval, aingle_value_get_long(src, &val));
			return aingle_value_set_long(dest, val);
		}

		case AINGLE_NULL:
		{
			check(rval, aingle_value_get_null(src));
			return aingle_value_set_null(dest);
		}

		case AINGLE_STRING:
		{
			aingle_wrapped_buffer_t  val;
			check(rval, aingle_value_grab_string(src, &val));
			return aingle_value_give_string_len(dest, &val);
		}

		case AINGLE_ARRAY:
		{
			size_t  count;
			check(rval, aingle_value_get_size(src, &count));

			size_t  i;
			for (i = 0; i < count; i++) {
				aingle_value_t  src_child;
				aingle_value_t  dest_child;

				check(rval, aingle_value_get_by_index
				      (src, i, &src_child, NULL));
				check(rval, aingle_value_append
				      (dest, &dest_child, NULL));
				check(rval, aingle_value_copy_fast
				      (&dest_child, &src_child));
			}

			return 0;
		}

		case AINGLE_ENUM:
		{
			int  val;
			check(rval, aingle_value_get_enum(src, &val));
			return aingle_value_set_enum(dest, val);
		}

		case AINGLE_FIXED:
		{
			aingle_wrapped_buffer_t  val;
			check(rval, aingle_value_grab_fixed(src, &val));
			return aingle_value_give_fixed(dest, &val);
		}

		case AINGLE_MAP:
		{
			size_t  count;
			check(rval, aingle_value_get_size(src, &count));

			size_t  i;
			for (i = 0; i < count; i++) {
				aingle_value_t  src_child;
				aingle_value_t  dest_child;
				const char  *key;

				check(rval, aingle_value_get_by_index
				      (src, i, &src_child, &key));
				check(rval, aingle_value_add
				      (dest, key, &dest_child, NULL, NULL));
				check(rval, aingle_value_copy_fast
				      (&dest_child, &src_child));
			}

			return 0;
		}

		case AINGLE_RECORD:
		{
			size_t  count;
			check(rval, aingle_value_get_size(src, &count));

			size_t  i;
			for (i = 0; i < count; i++) {
				aingle_value_t  src_child;
				aingle_value_t  dest_child;

				check(rval, aingle_value_get_by_index
				      (src, i, &src_child, NULL));
				check(rval, aingle_value_get_by_index
				      (dest, i, &dest_child, NULL));
				check(rval, aingle_value_copy_fast
				      (&dest_child, &src_child));
			}

			return 0;
		}

		case AINGLE_UNION:
		{
			int  disc;
			check(rval, aingle_value_get_discriminant(src, &disc));

			aingle_value_t  src_branch;
			aingle_value_t  dest_branch;

			check(rval, aingle_value_get_current_branch(src, &src_branch));
			check(rval, aingle_value_set_branch(dest, disc, &dest_branch));

			return aingle_value_copy_fast(&dest_branch, &src_branch);
		}

		default:
			return 0;
	}
}


int
aingle_value_copy(aingle_value_t *dest, const aingle_value_t *src)
{
	aingle_schema_t  dest_schema = aingle_value_get_schema(dest);
	aingle_schema_t  src_schema = aingle_value_get_schema(src);
	if (!aingle_schema_equal(dest_schema, src_schema)) {
		aingle_set_error("Schemas don't match");
		return EINVAL;
	}

	return aingle_value_copy_fast(dest, src);
}
