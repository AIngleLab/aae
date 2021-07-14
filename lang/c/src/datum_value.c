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
#include "aingle/errors.h"
#include "aingle/legacy.h"
#include "aingle/refcount.h"
#include "aingle/schema.h"
#include "aingle/value.h"
#include "aingle_private.h"

extern aingle_value_iface_t  AINGLE_DATUM_VALUE_CLASS;

aingle_value_iface_t *
aingle_datum_class(void)
{
	return &AINGLE_DATUM_VALUE_CLASS;
}

int
aingle_datum_as_value(aingle_value_t *value, aingle_datum_t src)
{
	value->iface = &AINGLE_DATUM_VALUE_CLASS;
	value->self = aingle_datum_incref(src);
	return 0;
}

static int
aingle_datum_as_child_value(aingle_value_t *value, aingle_datum_t src)
{
	value->iface = &AINGLE_DATUM_VALUE_CLASS;
	value->self = src;
	return 0;
}

static void
aingle_datum_value_incref(aingle_value_t *value)
{
	aingle_datum_t  self = (aingle_datum_t) value->self;
	aingle_datum_incref(self);
}

static void
aingle_datum_value_decref(aingle_value_t *value)
{
	aingle_datum_t  self = (aingle_datum_t) value->self;
	aingle_datum_decref(self);
}

static int
aingle_datum_value_reset(const aingle_value_iface_t *iface, void *vself)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_datum_reset(self);
}

static aingle_type_t
aingle_datum_value_get_type(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
#ifdef _WIN32
#pragma message("#warning: Bug: EINVAL is not of type aingle_type_t.")
#else
#warning "Bug: EINVAL is not of type aingle_type_t."
#endif
        /* We shouldn't use EINVAL as the return value to
         * check_param(), because EINVAL (= 22) is not a valid enum
         * aingle_type_t. This is a structural issue -- we would need a
         * different interface on all the get_type functions to fix
         * this. For now, suppressing the error by casting EINVAL to
         * (aingle_type_t) so the code compiles under C++.
         */
	check_param((aingle_type_t) EINVAL, self, "datum instance");
	return aingle_typeof(self);
}

static aingle_schema_t
aingle_datum_value_get_schema(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(NULL, self, "datum instance");
	return aingle_datum_get_schema(self);
}


static int
aingle_datum_value_get_boolean(const aingle_value_iface_t *iface,
			     const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	int8_t  value;
	check(rval, aingle_boolean_get(self, &value));
	*out = value;
	return 0;
}

static int
aingle_datum_value_get_bytes(const aingle_value_iface_t *iface,
			   const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	char  *bytes;
	int64_t  sz;
	check(rval, aingle_bytes_get(self, &bytes, &sz));
	if (buf != NULL) {
		*buf = (const void *) bytes;
	}
	if (size != NULL) {
		*size = sz;
	}
	return 0;
}

static int
aingle_datum_value_grab_bytes(const aingle_value_iface_t *iface,
			    const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	char  *bytes;
	int64_t  sz;
	check(rval, aingle_bytes_get(self, &bytes, &sz));

	/* nothing clever, just make a copy */
	return aingle_wrapped_buffer_new_copy(dest, bytes, sz);
}

static int
aingle_datum_value_get_double(const aingle_value_iface_t *iface,
			    const void *vself, double *out)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	double  value;
	check(rval, aingle_double_get(self, &value));
	*out = value;
	return 0;
}

static int
aingle_datum_value_get_float(const aingle_value_iface_t *iface,
			   const void *vself, float *out)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	float  value;
	check(rval, aingle_float_get(self, &value));
	*out = value;
	return 0;
}

static int
aingle_datum_value_get_int(const aingle_value_iface_t *iface,
			 const void *vself, int32_t *out)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	int32_t  value;
	check(rval, aingle_int32_get(self, &value));
	*out = value;
	return 0;
}

static int
aingle_datum_value_get_long(const aingle_value_iface_t *iface,
			  const void *vself, int64_t *out)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	int64_t  value;
	check(rval, aingle_int64_get(self, &value));
	*out = value;
	return 0;
}

static int
aingle_datum_value_get_null(const aingle_value_iface_t *iface,
			  const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, is_aingle_null(self), "datum instance");
	return 0;
}

static int
aingle_datum_value_get_string(const aingle_value_iface_t *iface,
			    const void *vself, const char **str, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	char  *value;
	check(rval, aingle_string_get(self, &value));
	if (str != NULL) {
		*str = (const char *) value;
	}
	if (size != NULL) {
		*size = strlen(value)+1;
	}
	return 0;
}

static int
aingle_datum_value_grab_string(const aingle_value_iface_t *iface,
			     const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	char  *str;
	size_t  sz;
	check(rval, aingle_string_get(self, &str));
	sz = strlen(str);

	/* nothing clever, just make a copy */
	/* sz doesn't contain NUL terminator */
	return aingle_wrapped_buffer_new_copy(dest, str, sz+1);
}

static int
aingle_datum_value_get_enum(const aingle_value_iface_t *iface,
			  const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, is_aingle_enum(self), "datum instance");
	*out = aingle_enum_get(self);
	return 0;
}

static int
aingle_datum_value_get_fixed(const aingle_value_iface_t *iface,
			   const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	char  *bytes;
	int64_t  sz;
	check(rval, aingle_fixed_get(self, &bytes, &sz));
	if (buf != NULL) {
		*buf = (const void *) bytes;
	}
	if (size != NULL) {
		*size = sz;
	}
	return 0;
}

static int
aingle_datum_value_grab_fixed(const aingle_value_iface_t *iface,
			    const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	char  *bytes;
	int64_t  sz;
	check(rval, aingle_fixed_get(self, &bytes, &sz));

	/* nothing clever, just make a copy */
	return aingle_wrapped_buffer_new_copy(dest, bytes, sz);
}


static int
aingle_datum_value_set_boolean(const aingle_value_iface_t *iface,
			     void *vself, int val)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_boolean_set(self, val);
}

static int
aingle_datum_value_set_bytes(const aingle_value_iface_t *iface,
			   void *vself, void *buf, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_bytes_set(self, (const char *) buf, size);
}

static int
aingle_datum_value_give_bytes(const aingle_value_iface_t *iface,
			    void *vself, aingle_wrapped_buffer_t *buf)
{
	/*
	 * We actually can't use aingle_givebytes_set, since it can't
	 * handle the extra free_ud parameter.  Ah well, this is
	 * deprecated, so go ahead and make a copy.
	 */

	int rval = aingle_datum_value_set_bytes
	    (iface, vself, (void *) buf->buf, buf->size);
	aingle_wrapped_buffer_free(buf);
	return rval;
}

static int
aingle_datum_value_set_double(const aingle_value_iface_t *iface,
			    void *vself, double val)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_double_set(self, val);
}

static int
aingle_datum_value_set_float(const aingle_value_iface_t *iface,
			   void *vself, float val)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_float_set(self, val);
}

static int
aingle_datum_value_set_int(const aingle_value_iface_t *iface,
			 void *vself, int32_t val)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_int32_set(self, val);
}

static int
aingle_datum_value_set_long(const aingle_value_iface_t *iface,
			  void *vself, int64_t val)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_int64_set(self, val);
}

static int
aingle_datum_value_set_null(const aingle_value_iface_t *iface, void *vself)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, is_aingle_null(self), "datum instance");
	return 0;
}

static int
aingle_datum_value_set_string(const aingle_value_iface_t *iface,
			    void *vself, const char *str)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_string_set(self, str);
}

static int
aingle_datum_value_set_string_len(const aingle_value_iface_t *iface,
				void *vself, const char *str, size_t size)
{
	AINGLE_UNUSED(iface);
	AINGLE_UNUSED(size);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_string_set(self, str);
}

static int
aingle_datum_value_give_string_len(const aingle_value_iface_t *iface,
				 void *vself, aingle_wrapped_buffer_t *buf)
{
	/*
	 * We actually can't use aingle_givestring_set, since it can't
	 * handle the extra free_ud parameter.  Ah well, this is
	 * deprecated, so go ahead and make a copy.
	 */

	int rval = aingle_datum_value_set_string_len
	    (iface, vself, (char *) buf->buf, buf->size-1);
	aingle_wrapped_buffer_free(buf);
	return rval;
}

static int
aingle_datum_value_set_enum(const aingle_value_iface_t *iface,
			  void *vself, int val)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_enum_set(self, val);
}

static int
aingle_datum_value_set_fixed(const aingle_value_iface_t *iface,
			   void *vself, void *buf, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");
	return aingle_fixed_set(self, (const char *) buf, size);
}

static int
aingle_datum_value_give_fixed(const aingle_value_iface_t *iface,
			    void *vself, aingle_wrapped_buffer_t *buf)
{
	/*
	 * We actually can't use aingle_givefixed_set, since it can't
	 * handle the extra free_ud parameter.  Ah well, this is
	 * deprecated, so go ahead and make a copy.
	 */

	int rval = aingle_datum_value_set_fixed
	    (iface, vself, (void *) buf->buf, buf->size);
	aingle_wrapped_buffer_free(buf);
	return rval;
}


static int
aingle_datum_value_get_size(const aingle_value_iface_t *iface,
			  const void *vself, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	if (is_aingle_array(self)) {
		*size = aingle_array_size(self);
		return 0;
	}

	if (is_aingle_map(self)) {
		*size = aingle_map_size(self);
		return 0;
	}

	if (is_aingle_record(self)) {
		aingle_schema_t  schema = aingle_datum_get_schema(self);
		*size = aingle_schema_record_size(schema);
		return 0;
	}

	aingle_set_error("Can only get size of array, map, or record, %d", aingle_typeof(self));
	return EINVAL;
}

static int
aingle_datum_value_get_by_index(const aingle_value_iface_t *iface,
			      const void *vself, size_t index,
			      aingle_value_t *child, const char **name)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	aingle_datum_t  child_datum;

	if (is_aingle_array(self)) {
		check(rval, aingle_array_get(self, index, &child_datum));
		return aingle_datum_as_child_value(child, child_datum);
	}

	if (is_aingle_map(self)) {
		const char  *real_key;
		check(rval, aingle_map_get_key(self, index, &real_key));
		if (name != NULL) {
			*name = real_key;
		}
		check(rval, aingle_map_get(self, real_key, &child_datum));
		return aingle_datum_as_child_value(child, child_datum);
	}

	if (is_aingle_record(self)) {
		aingle_schema_t  schema = aingle_datum_get_schema(self);
		const char  *field_name =
		    aingle_schema_record_field_name(schema, index);
		if (field_name == NULL) {
			return EINVAL;
		}
		if (name != NULL) {
			*name = field_name;
		}
		check(rval, aingle_record_get(self, field_name, &child_datum));
		return aingle_datum_as_child_value(child, child_datum);
	}

	aingle_set_error("Can only get by index from array, map, or record");
	return EINVAL;
}

static int
aingle_datum_value_get_by_name(const aingle_value_iface_t *iface,
			     const void *vself, const char *name,
			     aingle_value_t *child, size_t *index)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	int  rval;
	aingle_datum_t  child_datum;

	if (is_aingle_map(self)) {
		if (index != NULL) {
			int  real_index;
			check(rval, aingle_map_get_index(self, name, &real_index));
			*index = real_index;
		}

		check(rval, aingle_map_get(self, name, &child_datum));
		return aingle_datum_as_child_value(child, child_datum);
	}

	if (is_aingle_record(self)) {
		if (index != NULL) {
			aingle_schema_t  schema = aingle_datum_get_schema(self);
			*index = aingle_schema_record_field_get_index(schema, name);
		}

		check(rval, aingle_record_get(self, name, &child_datum));
		return aingle_datum_as_child_value(child, child_datum);
	}

	aingle_set_error("Can only get by name from map or record");
	return EINVAL;
}

static int
aingle_datum_value_get_discriminant(const aingle_value_iface_t *iface,
				  const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	if (!is_aingle_union(self)) {
		aingle_set_error("Can only get discriminant of union");
		return EINVAL;
	}

	*out = aingle_union_discriminant(self);
	return 0;
}

static int
aingle_datum_value_get_current_branch(const aingle_value_iface_t *iface,
				    const void *vself, aingle_value_t *branch)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	if (!is_aingle_union(self)) {
		aingle_set_error("Can only get current branch of union");
		return EINVAL;
	}

	aingle_datum_t  child_datum = aingle_union_current_branch(self);
	return aingle_datum_as_child_value(branch, child_datum);
}


static int
aingle_datum_value_append(const aingle_value_iface_t *iface,
			void *vself, aingle_value_t *child_out, size_t *new_index)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	if (!is_aingle_array(self)) {
		aingle_set_error("Can only append to array");
		return EINVAL;
	}

	int  rval;

	aingle_schema_t  array_schema = aingle_datum_get_schema(self);
	aingle_schema_t  child_schema = aingle_schema_array_items(array_schema);
	aingle_datum_t  child_datum = aingle_datum_from_schema(child_schema);
	if (child_datum == NULL) {
		return ENOMEM;
	}

	rval = aingle_array_append_datum(self, child_datum);
	aingle_datum_decref(child_datum);
	if (rval != 0) {
		return rval;
	}

	if (new_index != NULL) {
		*new_index = aingle_array_size(self) - 1;
	}
	return aingle_datum_as_child_value(child_out, child_datum);
}

static int
aingle_datum_value_add(const aingle_value_iface_t *iface,
		     void *vself, const char *key,
		     aingle_value_t *child, size_t *index, int *is_new)
{
	AINGLE_UNUSED(iface);
	aingle_datum_t  self = (aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	if (!is_aingle_map(self)) {
		aingle_set_error("Can only add to map");
		return EINVAL;
	}

	int  rval;
	aingle_datum_t  child_datum;

	if (aingle_map_get(self, key, &child_datum) == 0) {
		/* key already exists */
		if (is_new != NULL) {
			*is_new = 0;
		}
		if (index != NULL) {
			int  real_index;
			aingle_map_get_index(self, key, &real_index);
			*index = real_index;
		}
		return aingle_datum_as_child_value(child, child_datum);
	}

	/* key is new */
	aingle_schema_t  map_schema = aingle_datum_get_schema(self);
	aingle_schema_t  child_schema = aingle_schema_map_values(map_schema);
	child_datum = aingle_datum_from_schema(child_schema);
	if (child_datum == NULL) {
		return ENOMEM;
	}

	rval = aingle_map_set(self, key, child_datum);
	aingle_datum_decref(child_datum);
	if (rval != 0) {
		return rval;
	}

	if (is_new != NULL) {
		*is_new = 1;
	}
	if (index != NULL) {
		*index = aingle_map_size(self) - 1;
	}

	return aingle_datum_as_child_value(child, child_datum);
}

static int
aingle_datum_value_set_branch(const aingle_value_iface_t *iface,
			    void *vself, int discriminant,
			    aingle_value_t *branch)
{
	AINGLE_UNUSED(iface);
	const aingle_datum_t  self = (const aingle_datum_t) vself;
	check_param(EINVAL, self, "datum instance");

	if (!is_aingle_union(self)) {
		aingle_set_error("Can only set branch of union");
		return EINVAL;
	}

	int  rval;
	aingle_datum_t  child_datum;
	check(rval, aingle_union_set_discriminant(self, discriminant, &child_datum));
	return aingle_datum_as_child_value(branch, child_datum);
}


aingle_value_iface_t  AINGLE_DATUM_VALUE_CLASS =
{
	/* "class" methods */
	NULL, /* incref */
	NULL, /* decref */
	/* general "instance" methods */
	aingle_datum_value_incref,
	aingle_datum_value_decref,
	aingle_datum_value_reset,
	aingle_datum_value_get_type,
	aingle_datum_value_get_schema,
	/* primitive getters */
	aingle_datum_value_get_boolean,
	aingle_datum_value_get_bytes,
	aingle_datum_value_grab_bytes,
	aingle_datum_value_get_double,
	aingle_datum_value_get_float,
	aingle_datum_value_get_int,
	aingle_datum_value_get_long,
	aingle_datum_value_get_null,
	aingle_datum_value_get_string,
	aingle_datum_value_grab_string,
	aingle_datum_value_get_enum,
	aingle_datum_value_get_fixed,
	aingle_datum_value_grab_fixed,
	/* primitive setters */
	aingle_datum_value_set_boolean,
	aingle_datum_value_set_bytes,
	aingle_datum_value_give_bytes,
	aingle_datum_value_set_double,
	aingle_datum_value_set_float,
	aingle_datum_value_set_int,
	aingle_datum_value_set_long,
	aingle_datum_value_set_null,
	aingle_datum_value_set_string,
	aingle_datum_value_set_string_len,
	aingle_datum_value_give_string_len,
	aingle_datum_value_set_enum,
	aingle_datum_value_set_fixed,
	aingle_datum_value_give_fixed,
	/* compound getters */
	aingle_datum_value_get_size,
	aingle_datum_value_get_by_index,
	aingle_datum_value_get_by_name,
	aingle_datum_value_get_discriminant,
	aingle_datum_value_get_current_branch,
	/* compound setters */
	aingle_datum_value_append,
	aingle_datum_value_add,
	aingle_datum_value_set_branch
};
