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

#include "aingle/allocation.h"
#include "aingle/basics.h"
#include "aingle/errors.h"
#include "aingle/legacy.h"
#include "aingle/refcount.h"
#include "aingle/schema.h"
#include "aingle_private.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "datum.h"
#include "schema.h"
#include "encoding.h"

#define DEFAULT_TABLE_SIZE 32

static void aingle_datum_init(aingle_datum_t datum, aingle_type_t type)
{
	datum->type = type;
	datum->class_type = AINGLE_DATUM;
	aingle_refcount_set(&datum->refcount, 1);
}

static void
aingle_str_free_wrapper(void *ptr, size_t sz)
{
	// don't need sz, since the size is stored in the string buffer
	AINGLE_UNUSED(sz);
	aingle_str_free((char *)ptr);
}

static aingle_datum_t aingle_string_private(char *str, int64_t size,
					aingle_free_func_t string_free)
{
	struct aingle_string_datum_t *datum =
	    (struct aingle_string_datum_t *) aingle_new(struct aingle_string_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new string datum");
		return NULL;
	}
	datum->s = str;
	datum->size = size;
	datum->free = string_free;

	aingle_datum_init(&datum->obj, AINGLE_STRING);
	return &datum->obj;
}

aingle_datum_t aingle_string(const char *str)
{
	char *p = aingle_strdup(str);
	if (!p) {
		aingle_set_error("Cannot copy string content");
		return NULL;
	}
	aingle_datum_t s_datum = aingle_string_private(p, 0, aingle_str_free_wrapper);
	if (!s_datum) {
		aingle_str_free(p);
	}

	return s_datum;
}

aingle_datum_t aingle_givestring(const char *str,
			     aingle_free_func_t free)
{
	int64_t  sz = strlen(str)+1;
	return aingle_string_private((char *)str, sz, free);
}

int aingle_string_get(aingle_datum_t datum, char **p)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_string(datum), "string datum");
	check_param(EINVAL, p, "string buffer");

	*p = aingle_datum_to_string(datum)->s;
	return 0;
}

static int aingle_string_set_private(aingle_datum_t datum,
	       			   const char *p, int64_t size,
				   aingle_free_func_t string_free)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_string(datum), "string datum");
	check_param(EINVAL, p, "string content");

	struct aingle_string_datum_t *string = aingle_datum_to_string(datum);
	if (string->free) {
		string->free(string->s, string->size);
	}
	string->free = string_free;
	string->s = (char *)p;
	string->size = size;
	return 0;
}

int aingle_string_set(aingle_datum_t datum, const char *p)
{
	char *string_copy = aingle_strdup(p);
	int rval;
	if (!string_copy) {
		aingle_set_error("Cannot copy string content");
		return ENOMEM;
	}
	rval = aingle_string_set_private(datum, string_copy, 0,
				       aingle_str_free_wrapper);
	if (rval) {
		aingle_str_free(string_copy);
	}
	return rval;
}

int aingle_givestring_set(aingle_datum_t datum, const char *p,
			aingle_free_func_t free)
{
	int64_t  size = strlen(p)+1;
	return aingle_string_set_private(datum, p, size, free);
}

static aingle_datum_t aingle_bytes_private(char *bytes, int64_t size,
				       aingle_free_func_t bytes_free)
{
	struct aingle_bytes_datum_t *datum;
	datum = (struct aingle_bytes_datum_t *) aingle_new(struct aingle_bytes_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new bytes datum");
		return NULL;
	}
	datum->bytes = bytes;
	datum->size = size;
	datum->free = bytes_free;

	aingle_datum_init(&datum->obj, AINGLE_BYTES);
	return &datum->obj;
}

aingle_datum_t aingle_bytes(const char *bytes, int64_t size)
{
	char *bytes_copy = (char *) aingle_malloc(size);
	if (!bytes_copy) {
		aingle_set_error("Cannot copy bytes content");
		return NULL;
	}
	memcpy(bytes_copy, bytes, size);
	aingle_datum_t  result =
		aingle_bytes_private(bytes_copy, size, aingle_alloc_free_func);
	if (result == NULL) {
		aingle_free(bytes_copy, size);
	}
	return result;
}

aingle_datum_t aingle_givebytes(const char *bytes, int64_t size,
			    aingle_free_func_t free)
{
	return aingle_bytes_private((char *)bytes, size, free);
}

static int aingle_bytes_set_private(aingle_datum_t datum, const char *bytes,
				  const int64_t size,
				  aingle_free_func_t bytes_free)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_bytes(datum), "bytes datum");

	struct aingle_bytes_datum_t *b = aingle_datum_to_bytes(datum);
	if (b->free) {
		b->free(b->bytes, b->size);
	}

	b->free = bytes_free;
	b->bytes = (char *)bytes;
	b->size = size;
	return 0;
}

int aingle_bytes_set(aingle_datum_t datum, const char *bytes, const int64_t size)
{
	int rval;
	char *bytes_copy = (char *) aingle_malloc(size);
	if (!bytes_copy) {
		aingle_set_error("Cannot copy bytes content");
		return ENOMEM;
	}
	memcpy(bytes_copy, bytes, size);
	rval = aingle_bytes_set_private(datum, bytes_copy, size, aingle_alloc_free_func);
	if (rval) {
		aingle_free(bytes_copy, size);
	}
	return rval;
}

int aingle_givebytes_set(aingle_datum_t datum, const char *bytes,
		       const int64_t size, aingle_free_func_t free)
{
	return aingle_bytes_set_private(datum, bytes, size, free);
}

int aingle_bytes_get(aingle_datum_t datum, char **bytes, int64_t * size)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_bytes(datum), "bytes datum");
	check_param(EINVAL, bytes, "bytes");
	check_param(EINVAL, size, "size");

	*bytes = aingle_datum_to_bytes(datum)->bytes;
	*size = aingle_datum_to_bytes(datum)->size;
	return 0;
}

aingle_datum_t aingle_int32(int32_t i)
{
	struct aingle_int32_datum_t *datum =
	    (struct aingle_int32_datum_t *) aingle_new(struct aingle_int32_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new int datum");
		return NULL;
	}
	datum->i32 = i;

	aingle_datum_init(&datum->obj, AINGLE_INT32);
	return &datum->obj;
}

int aingle_int32_get(aingle_datum_t datum, int32_t * i)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_int32(datum), "int datum");
	check_param(EINVAL, i, "value pointer");

	*i = aingle_datum_to_int32(datum)->i32;
	return 0;
}

int aingle_int32_set(aingle_datum_t datum, const int32_t i)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_int32(datum), "int datum");

	aingle_datum_to_int32(datum)->i32 = i;
	return 0;
}

aingle_datum_t aingle_int64(int64_t l)
{
	struct aingle_int64_datum_t *datum =
	    (struct aingle_int64_datum_t *) aingle_new(struct aingle_int64_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new long datum");
		return NULL;
	}
	datum->i64 = l;

	aingle_datum_init(&datum->obj, AINGLE_INT64);
	return &datum->obj;
}

int aingle_int64_get(aingle_datum_t datum, int64_t * l)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_int64(datum), "long datum");
	check_param(EINVAL, l, "value pointer");

	*l = aingle_datum_to_int64(datum)->i64;
	return 0;
}

int aingle_int64_set(aingle_datum_t datum, const int64_t l)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_int64(datum), "long datum");

	aingle_datum_to_int64(datum)->i64 = l;
	return 0;
}

aingle_datum_t aingle_float(float f)
{
	struct aingle_float_datum_t *datum =
	    (struct aingle_float_datum_t *) aingle_new(struct aingle_float_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new float datum");
		return NULL;
	}
	datum->f = f;

	aingle_datum_init(&datum->obj, AINGLE_FLOAT);
	return &datum->obj;
}

int aingle_float_set(aingle_datum_t datum, const float f)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_float(datum), "float datum");

	aingle_datum_to_float(datum)->f = f;
	return 0;
}

int aingle_float_get(aingle_datum_t datum, float *f)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_float(datum), "float datum");
	check_param(EINVAL, f, "value pointer");

	*f = aingle_datum_to_float(datum)->f;
	return 0;
}

aingle_datum_t aingle_double(double d)
{
	struct aingle_double_datum_t *datum =
	    (struct aingle_double_datum_t *) aingle_new(struct aingle_double_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new double atom");
		return NULL;
	}
	datum->d = d;

	aingle_datum_init(&datum->obj, AINGLE_DOUBLE);
	return &datum->obj;
}

int aingle_double_set(aingle_datum_t datum, const double d)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_double(datum), "double datum");

	aingle_datum_to_double(datum)->d = d;
	return 0;
}

int aingle_double_get(aingle_datum_t datum, double *d)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_double(datum), "double datum");
	check_param(EINVAL, d, "value pointer");

	*d = aingle_datum_to_double(datum)->d;
	return 0;
}

aingle_datum_t aingle_boolean(int8_t i)
{
	struct aingle_boolean_datum_t *datum =
	    (struct aingle_boolean_datum_t *) aingle_new(struct aingle_boolean_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new boolean datum");
		return NULL;
	}
	datum->i = i;
	aingle_datum_init(&datum->obj, AINGLE_BOOLEAN);
	return &datum->obj;
}

int aingle_boolean_set(aingle_datum_t datum, const int8_t i)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_boolean(datum), "boolean datum");

	aingle_datum_to_boolean(datum)->i = i;
	return 0;
}

int aingle_boolean_get(aingle_datum_t datum, int8_t * i)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_boolean(datum), "boolean datum");
	check_param(EINVAL, i, "value pointer");

	*i = aingle_datum_to_boolean(datum)->i;
	return 0;
}

aingle_datum_t aingle_null(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_NULL,
		AINGLE_DATUM,
		1
	};
	return aingle_datum_incref(&obj);
}

aingle_datum_t aingle_union(aingle_schema_t schema,
			int64_t discriminant, aingle_datum_t value)
{
	check_param(NULL, is_aingle_schema(schema), "schema");

	struct aingle_union_datum_t *datum =
	    (struct aingle_union_datum_t *) aingle_new(struct aingle_union_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new union datum");
		return NULL;
	}
	datum->schema = aingle_schema_incref(schema);
	datum->discriminant = discriminant;
	datum->value = aingle_datum_incref(value);

	aingle_datum_init(&datum->obj, AINGLE_UNION);
	return &datum->obj;
}

int64_t aingle_union_discriminant(const aingle_datum_t datum)
{
	return aingle_datum_to_union(datum)->discriminant;
}

aingle_datum_t aingle_union_current_branch(aingle_datum_t datum)
{
	return aingle_datum_to_union(datum)->value;
}

int aingle_union_set_discriminant(aingle_datum_t datum,
				int discriminant,
				aingle_datum_t *branch)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_union(datum), "union datum");

	struct aingle_union_datum_t  *unionp = aingle_datum_to_union(datum);

	aingle_schema_t  schema = unionp->schema;
	aingle_schema_t  branch_schema =
	    aingle_schema_union_branch(schema, discriminant);

	if (branch_schema == NULL) {
		// That branch doesn't exist!
		aingle_set_error("Branch %d doesn't exist", discriminant);
		return EINVAL;
	}

	if (unionp->discriminant != discriminant) {
		// If we're changing the branch, throw away any old
		// branch value.
		if (unionp->value != NULL) {
			aingle_datum_decref(unionp->value);
			unionp->value = NULL;
		}

		unionp->discriminant = discriminant;
	}

	// Create a new branch value, if there isn't one already.
	if (unionp->value == NULL) {
		unionp->value = aingle_datum_from_schema(branch_schema);
	}

	if (branch != NULL) {
		*branch = unionp->value;
	}

	return 0;
}

aingle_datum_t aingle_record(aingle_schema_t schema)
{
	check_param(NULL, is_aingle_schema(schema), "schema");

	struct aingle_record_datum_t *datum =
	    (struct aingle_record_datum_t *) aingle_new(struct aingle_record_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new record datum");
		return NULL;
	}
	datum->field_order = st_init_numtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->field_order) {
		aingle_set_error("Cannot create new record datum");
		aingle_freet(struct aingle_record_datum_t, datum);
		return NULL;
	}
	datum->fields_byname = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->fields_byname) {
		aingle_set_error("Cannot create new record datum");
		st_free_table(datum->field_order);
		aingle_freet(struct aingle_record_datum_t, datum);
		return NULL;
	}

	datum->schema = aingle_schema_incref(schema);
	aingle_datum_init(&datum->obj, AINGLE_RECORD);
	return &datum->obj;
}

int
aingle_record_get(const aingle_datum_t datum, const char *field_name,
		aingle_datum_t * field)
{
	union {
		aingle_datum_t field;
		st_data_t data;
	} val;
	if (is_aingle_datum(datum) && is_aingle_record(datum) && field_name) {
		if (st_lookup
		    (aingle_datum_to_record(datum)->fields_byname,
		     (st_data_t) field_name, &(val.data))) {
			*field = val.field;
			return 0;
		}
	}
	aingle_set_error("No field named %s", field_name);
	return EINVAL;
}

int
aingle_record_set(aingle_datum_t datum, const char *field_name,
		const aingle_datum_t field_value)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_record(datum), "record datum");
	check_param(EINVAL, field_name, "field_name");

	char *key = (char *)field_name;
	aingle_datum_t old_field;

	if (aingle_record_get(datum, field_name, &old_field) == 0) {
		/* Overriding old value */
		aingle_datum_decref(old_field);
	} else {
		/* Inserting new value */
		struct aingle_record_datum_t *record =
		    aingle_datum_to_record(datum);
		key = aingle_strdup(field_name);
		if (!key) {
			aingle_set_error("Cannot copy field name");
			return ENOMEM;
		}
		st_insert(record->field_order,
			  record->field_order->num_entries,
			  (st_data_t) key);
	}
	aingle_datum_incref(field_value);
	st_insert(aingle_datum_to_record(datum)->fields_byname,
		  (st_data_t) key, (st_data_t) field_value);
	return 0;
}

aingle_datum_t aingle_enum(aingle_schema_t schema, int i)
{
	check_param(NULL, is_aingle_schema(schema), "schema");

	struct aingle_enum_datum_t *datum =
	    (struct aingle_enum_datum_t *) aingle_new(struct aingle_enum_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new enum datum");
		return NULL;
	}
	datum->schema = aingle_schema_incref(schema);
	datum->value = i;

	aingle_datum_init(&datum->obj, AINGLE_ENUM);
	return &datum->obj;
}

int aingle_enum_get(const aingle_datum_t datum)
{
	return aingle_datum_to_enum(datum)->value;
}

const char *aingle_enum_get_name(const aingle_datum_t datum)
{
	int  value = aingle_enum_get(datum);
	aingle_schema_t  schema = aingle_datum_to_enum(datum)->schema;
	return aingle_schema_enum_get(schema, value);
}

int aingle_enum_set(aingle_datum_t datum, const int symbol_value)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_enum(datum), "enum datum");

	aingle_datum_to_enum(datum)->value = symbol_value;
	return 0;
}

int aingle_enum_set_name(aingle_datum_t datum, const char *symbol_name)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_enum(datum), "enum datum");
	check_param(EINVAL, symbol_name, "symbol name");

	aingle_schema_t  schema = aingle_datum_to_enum(datum)->schema;
	int  symbol_value = aingle_schema_enum_get_by_name(schema, symbol_name);
	if (symbol_value == -1) {
		aingle_set_error("No symbol named %s", symbol_name);
		return EINVAL;
	}
	aingle_datum_to_enum(datum)->value = symbol_value;
	return 0;
}

static aingle_datum_t aingle_fixed_private(aingle_schema_t schema,
				       const char *bytes, const int64_t size,
				       aingle_free_func_t fixed_free)
{
	check_param(NULL, is_aingle_schema(schema), "schema");
	struct aingle_fixed_schema_t *fschema = aingle_schema_to_fixed(schema);
	if (size != fschema->size) {
		aingle_free((char *) bytes, size);
		aingle_set_error("Fixed size (%zu) doesn't match schema (%zu)",
			       (size_t) size, (size_t) fschema->size);
		return NULL;
	}

	struct aingle_fixed_datum_t *datum =
	    (struct aingle_fixed_datum_t *) aingle_new(struct aingle_fixed_datum_t);
	if (!datum) {
		aingle_free((char *) bytes, size);
		aingle_set_error("Cannot create new fixed datum");
		return NULL;
	}
	datum->schema = aingle_schema_incref(schema);
	datum->size = size;
	datum->bytes = (char *)bytes;
	datum->free = fixed_free;

	aingle_datum_init(&datum->obj, AINGLE_FIXED);
	return &datum->obj;
}

aingle_datum_t aingle_fixed(aingle_schema_t schema,
			const char *bytes, const int64_t size)
{
	char *bytes_copy = (char *) aingle_malloc(size);
	if (!bytes_copy) {
		aingle_set_error("Cannot copy fixed content");
		return NULL;
	}
	memcpy(bytes_copy, bytes, size);
	return aingle_fixed_private(schema, bytes_copy, size, aingle_alloc_free_func);
}

aingle_datum_t aingle_givefixed(aingle_schema_t schema,
			    const char *bytes, const int64_t size,
			    aingle_free_func_t free)
{
	return aingle_fixed_private(schema, bytes, size, free);
}

static int aingle_fixed_set_private(aingle_datum_t datum,
				  const char *bytes, const int64_t size,
				  aingle_free_func_t fixed_free)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_fixed(datum), "fixed datum");

	struct aingle_fixed_datum_t *fixed = aingle_datum_to_fixed(datum);
	struct aingle_fixed_schema_t *schema = aingle_schema_to_fixed(fixed->schema);
	if (size != schema->size) {
		aingle_set_error("Fixed size doesn't match schema");
		return EINVAL;
	}

	if (fixed->free) {
		fixed->free(fixed->bytes, fixed->size);
	}

	fixed->free = fixed_free;
	fixed->bytes = (char *)bytes;
	fixed->size = size;
	return 0;
}

int aingle_fixed_set(aingle_datum_t datum, const char *bytes, const int64_t size)
{
	int rval;
	char *bytes_copy = (char *) aingle_malloc(size);
	if (!bytes_copy) {
		aingle_set_error("Cannot copy fixed content");
		return ENOMEM;
	}
	memcpy(bytes_copy, bytes, size);
	rval = aingle_fixed_set_private(datum, bytes_copy, size, aingle_alloc_free_func);
	if (rval) {
		aingle_free(bytes_copy, size);
	}
	return rval;
}

int aingle_givefixed_set(aingle_datum_t datum, const char *bytes,
		       const int64_t size, aingle_free_func_t free)
{
	return aingle_fixed_set_private(datum, bytes, size, free);
}

int aingle_fixed_get(aingle_datum_t datum, char **bytes, int64_t * size)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_fixed(datum), "fixed datum");
	check_param(EINVAL, bytes, "bytes");
	check_param(EINVAL, size, "size");

	*bytes = aingle_datum_to_fixed(datum)->bytes;
	*size = aingle_datum_to_fixed(datum)->size;
	return 0;
}

static int
aingle_init_map(struct aingle_map_datum_t *datum)
{
	datum->map = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->map) {
		aingle_set_error("Cannot create new map datum");
		return ENOMEM;
	}
	datum->indices_by_key = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->indices_by_key) {
		aingle_set_error("Cannot create new map datum");
		st_free_table(datum->map);
		return ENOMEM;
	}
	datum->keys_by_index = st_init_numtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->keys_by_index) {
		aingle_set_error("Cannot create new map datum");
		st_free_table(datum->indices_by_key);
		st_free_table(datum->map);
		return ENOMEM;
	}
	return 0;
}

aingle_datum_t aingle_map(aingle_schema_t schema)
{
	check_param(NULL, is_aingle_schema(schema), "schema");

	struct aingle_map_datum_t *datum =
	    (struct aingle_map_datum_t *) aingle_new(struct aingle_map_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new map datum");
		return NULL;
	}

	if (aingle_init_map(datum) != 0) {
		aingle_freet(struct aingle_map_datum_t, datum);
		return NULL;
	}

	datum->schema = aingle_schema_incref(schema);
	aingle_datum_init(&datum->obj, AINGLE_MAP);
	return &datum->obj;
}

size_t
aingle_map_size(const aingle_datum_t datum)
{
	const struct aingle_map_datum_t  *map = aingle_datum_to_map(datum);
	return map->map->num_entries;
}

int
aingle_map_get(const aingle_datum_t datum, const char *key, aingle_datum_t * value)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_map(datum), "map datum");
	check_param(EINVAL, key, "key");
	check_param(EINVAL, value, "value");

	union {
		aingle_datum_t datum;
		st_data_t data;
	} val;

	struct aingle_map_datum_t *map = aingle_datum_to_map(datum);
	if (st_lookup(map->map, (st_data_t) key, &(val.data))) {
		*value = val.datum;
		return 0;
	}

	aingle_set_error("No map element named %s", key);
	return EINVAL;
}

int aingle_map_get_key(const aingle_datum_t datum, int index,
		     const char **key)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_map(datum), "map datum");
	check_param(EINVAL, index >= 0, "index");
	check_param(EINVAL, key, "key");

	union {
		st_data_t data;
		char *key;
	} val;

	struct aingle_map_datum_t *map = aingle_datum_to_map(datum);
	if (st_lookup(map->keys_by_index, (st_data_t) index, &val.data)) {
		*key = val.key;
		return 0;
	}

	aingle_set_error("No map element with index %d", index);
	return EINVAL;
}

int aingle_map_get_index(const aingle_datum_t datum, const char *key,
		       int *index)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_map(datum), "map datum");
	check_param(EINVAL, key, "key");
	check_param(EINVAL, index, "index");

	st_data_t  data;

	struct aingle_map_datum_t *map = aingle_datum_to_map(datum);
	if (st_lookup(map->indices_by_key, (st_data_t) key, &data)) {
		*index = (int) data;
		return 0;
	}

	aingle_set_error("No map element with key %s", key);
	return EINVAL;
}

int
aingle_map_set(aingle_datum_t datum, const char *key,
	     const aingle_datum_t value)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	check_param(EINVAL, is_aingle_map(datum), "map datum");
	check_param(EINVAL, key, "key");
	check_param(EINVAL, is_aingle_datum(value), "value");

	char *save_key = (char *)key;
	aingle_datum_t old_datum;

	struct aingle_map_datum_t  *map = aingle_datum_to_map(datum);

	if (aingle_map_get(datum, key, &old_datum) == 0) {
		/* Overwriting an old value */
		aingle_datum_decref(old_datum);
	} else {
		/* Inserting a new value */
		save_key = aingle_strdup(key);
		if (!save_key) {
			aingle_set_error("Cannot copy map key");
			return ENOMEM;
		}
		int  new_index = map->map->num_entries;
		st_insert(map->indices_by_key, (st_data_t) save_key,
			  (st_data_t) new_index);
		st_insert(map->keys_by_index, (st_data_t) new_index,
			  (st_data_t) save_key);
	}
	aingle_datum_incref(value);
	st_insert(map->map, (st_data_t) save_key, (st_data_t) value);
	return 0;
}

static int
aingle_init_array(struct aingle_array_datum_t *datum)
{
	datum->els = st_init_numtable_with_size(DEFAULT_TABLE_SIZE);
	if (!datum->els) {
		aingle_set_error("Cannot create new array datum");
		return ENOMEM;
	}
	return 0;
}

aingle_datum_t aingle_array(aingle_schema_t schema)
{
	check_param(NULL, is_aingle_schema(schema), "schema");

	struct aingle_array_datum_t *datum =
	    (struct aingle_array_datum_t *) aingle_new(struct aingle_array_datum_t);
	if (!datum) {
		aingle_set_error("Cannot create new array datum");
		return NULL;
	}

	if (aingle_init_array(datum) != 0) {
		aingle_freet(struct aingle_array_datum_t, datum);
		return NULL;
	}

	datum->schema = aingle_schema_incref(schema);
	aingle_datum_init(&datum->obj, AINGLE_ARRAY);
	return &datum->obj;
}

int
aingle_array_get(const aingle_datum_t array_datum, int64_t index, aingle_datum_t * value)
{
	check_param(EINVAL, is_aingle_datum(array_datum), "datum");
	check_param(EINVAL, is_aingle_array(array_datum), "array datum");
	check_param(EINVAL, value, "value pointer");

	union {
		st_data_t data;
		aingle_datum_t datum;
	} val;

        const struct aingle_array_datum_t * array = aingle_datum_to_array(array_datum);
	if (st_lookup(array->els, index, &val.data)) {
		*value = val.datum;
		return 0;
	}

	aingle_set_error("No array element with index %ld", (long) index);
	return EINVAL;
}

size_t
aingle_array_size(const aingle_datum_t datum)
{
	const struct aingle_array_datum_t  *array = aingle_datum_to_array(datum);
	return array->els->num_entries;
}

int
aingle_array_append_datum(aingle_datum_t array_datum,
			const aingle_datum_t datum)
{
	check_param(EINVAL, is_aingle_datum(array_datum), "datum");
	check_param(EINVAL, is_aingle_array(array_datum), "array datum");
	check_param(EINVAL, is_aingle_datum(datum), "element datum");

	struct aingle_array_datum_t *array = aingle_datum_to_array(array_datum);
	st_insert(array->els, array->els->num_entries,
		  (st_data_t) aingle_datum_incref(datum));
	return 0;
}

static int char_datum_free_foreach(char *key, aingle_datum_t datum, void *arg)
{
	AINGLE_UNUSED(arg);

	aingle_datum_decref(datum);
	aingle_str_free(key);
	return ST_DELETE;
}

static int array_free_foreach(int i, aingle_datum_t datum, void *arg)
{
	AINGLE_UNUSED(i);
	AINGLE_UNUSED(arg);

	aingle_datum_decref(datum);
	return ST_DELETE;
}

aingle_schema_t aingle_datum_get_schema(const aingle_datum_t datum)
{
	check_param(NULL, is_aingle_datum(datum), "datum");

	switch (aingle_typeof(datum)) {
		/*
		 * For the primitive types, which don't store an
		 * explicit reference to their schema, we decref the
		 * schema before returning.  This maintains the
		 * invariant that this function doesn't add any
		 * additional references to the schema.  The primitive
		 * schemas won't be freed, because there's always at
		 * least 1 reference for their initial static
		 * initializers.
		 */

		case AINGLE_STRING:
			{
				aingle_schema_t  result = aingle_schema_string();
				aingle_schema_decref(result);
				return result;
			}
		case AINGLE_BYTES:
			{
				aingle_schema_t  result = aingle_schema_bytes();
				aingle_schema_decref(result);
				return result;
			}
		case AINGLE_INT32:
			{
				aingle_schema_t  result = aingle_schema_int();
				aingle_schema_decref(result);
				return result;
			}
		case AINGLE_INT64:
			{
				aingle_schema_t  result = aingle_schema_long();
				aingle_schema_decref(result);
				return result;
			}
		case AINGLE_FLOAT:
			{
				aingle_schema_t  result = aingle_schema_float();
				aingle_schema_decref(result);
				return result;
			}
		case AINGLE_DOUBLE:
			{
				aingle_schema_t  result = aingle_schema_double();
				aingle_schema_decref(result);
				return result;
			}
		case AINGLE_BOOLEAN:
			{
				aingle_schema_t  result = aingle_schema_boolean();
				aingle_schema_decref(result);
				return result;
			}
		case AINGLE_NULL:
			{
				aingle_schema_t  result = aingle_schema_null();
				aingle_schema_decref(result);
				return result;
			}

		case AINGLE_RECORD:
			return aingle_datum_to_record(datum)->schema;
		case AINGLE_ENUM:
			return aingle_datum_to_enum(datum)->schema;
		case AINGLE_FIXED:
			return aingle_datum_to_fixed(datum)->schema;
		case AINGLE_MAP:
			return aingle_datum_to_map(datum)->schema;
		case AINGLE_ARRAY:
			return aingle_datum_to_array(datum)->schema;
		case AINGLE_UNION:
			return aingle_datum_to_union(datum)->schema;

		default:
			return NULL;
	}
}

static void aingle_datum_free(aingle_datum_t datum)
{
	if (is_aingle_datum(datum)) {
		switch (aingle_typeof(datum)) {
		case AINGLE_STRING:{
				struct aingle_string_datum_t *string;
				string = aingle_datum_to_string(datum);
				if (string->free) {
					string->free(string->s, string->size);
				}
				aingle_freet(struct aingle_string_datum_t, string);
			}
			break;
		case AINGLE_BYTES:{
				struct aingle_bytes_datum_t *bytes;
				bytes = aingle_datum_to_bytes(datum);
				if (bytes->free) {
					bytes->free(bytes->bytes, bytes->size);
				}
				aingle_freet(struct aingle_bytes_datum_t, bytes);
			}
			break;
		case AINGLE_INT32:{
				aingle_freet(struct aingle_int32_datum_t, datum);
			}
			break;
		case AINGLE_INT64:{
				aingle_freet(struct aingle_int64_datum_t, datum);
			}
			break;
		case AINGLE_FLOAT:{
				aingle_freet(struct aingle_float_datum_t, datum);
			}
			break;
		case AINGLE_DOUBLE:{
				aingle_freet(struct aingle_double_datum_t, datum);
			}
			break;
		case AINGLE_BOOLEAN:{
				aingle_freet(struct aingle_boolean_datum_t, datum);
			}
			break;
		case AINGLE_NULL:
			/* Nothing allocated */
			break;

		case AINGLE_RECORD:{
				struct aingle_record_datum_t *record;
				record = aingle_datum_to_record(datum);
				aingle_schema_decref(record->schema);
				st_foreach(record->fields_byname,
					   HASH_FUNCTION_CAST char_datum_free_foreach, 0);
				st_free_table(record->field_order);
				st_free_table(record->fields_byname);
				aingle_freet(struct aingle_record_datum_t, record);
			}
			break;
		case AINGLE_ENUM:{
				struct aingle_enum_datum_t *enump;
				enump = aingle_datum_to_enum(datum);
				aingle_schema_decref(enump->schema);
				aingle_freet(struct aingle_enum_datum_t, enump);
			}
			break;
		case AINGLE_FIXED:{
				struct aingle_fixed_datum_t *fixed;
				fixed = aingle_datum_to_fixed(datum);
				aingle_schema_decref(fixed->schema);
				if (fixed->free) {
					fixed->free((void *)fixed->bytes,
						    fixed->size);
				}
				aingle_freet(struct aingle_fixed_datum_t, fixed);
			}
			break;
		case AINGLE_MAP:{
				struct aingle_map_datum_t *map;
				map = aingle_datum_to_map(datum);
				aingle_schema_decref(map->schema);
				st_foreach(map->map, HASH_FUNCTION_CAST char_datum_free_foreach,
					   0);
				st_free_table(map->map);
				st_free_table(map->indices_by_key);
				st_free_table(map->keys_by_index);
				aingle_freet(struct aingle_map_datum_t, map);
			}
			break;
		case AINGLE_ARRAY:{
				struct aingle_array_datum_t *array;
				array = aingle_datum_to_array(datum);
				aingle_schema_decref(array->schema);
				st_foreach(array->els, HASH_FUNCTION_CAST array_free_foreach, 0);
				st_free_table(array->els);
				aingle_freet(struct aingle_array_datum_t, array);
			}
			break;
		case AINGLE_UNION:{
				struct aingle_union_datum_t *unionp;
				unionp = aingle_datum_to_union(datum);
				aingle_schema_decref(unionp->schema);
				aingle_datum_decref(unionp->value);
				aingle_freet(struct aingle_union_datum_t, unionp);
			}
			break;
		case AINGLE_LINK:{
				/* TODO */
			}
			break;
		}
	}
}

static int
datum_reset_foreach(int i, aingle_datum_t datum, void *arg)
{
	AINGLE_UNUSED(i);
	int  rval;
	int  *result = (int *) arg;

	rval = aingle_datum_reset(datum);
	if (rval == 0) {
		return ST_CONTINUE;
	} else {
		*result = rval;
		return ST_STOP;
	}
}

int
aingle_datum_reset(aingle_datum_t datum)
{
	check_param(EINVAL, is_aingle_datum(datum), "datum");
	int  rval;

	switch (aingle_typeof(datum)) {
		case AINGLE_ARRAY:
		{
			struct aingle_array_datum_t *array;
			array = aingle_datum_to_array(datum);
			st_foreach(array->els, HASH_FUNCTION_CAST array_free_foreach, 0);
			st_free_table(array->els);

			rval = aingle_init_array(array);
			if (rval != 0) {
				aingle_freet(struct aingle_array_datum_t, array);
				return rval;
			}
			return 0;
		}

		case AINGLE_MAP:
		{
			struct aingle_map_datum_t *map;
			map = aingle_datum_to_map(datum);
			st_foreach(map->map, HASH_FUNCTION_CAST char_datum_free_foreach, 0);
			st_free_table(map->map);
			st_free_table(map->indices_by_key);
			st_free_table(map->keys_by_index);

			rval = aingle_init_map(map);
			if (rval != 0) {
				aingle_freet(struct aingle_map_datum_t, map);
				return rval;
			}
			return 0;
		}

		case AINGLE_RECORD:
		{
			struct aingle_record_datum_t *record;
			record = aingle_datum_to_record(datum);
			rval = 0;
			st_foreach(record->fields_byname,
				   HASH_FUNCTION_CAST datum_reset_foreach, (st_data_t) &rval);
			return rval;
		}

		case AINGLE_UNION:
		{
			struct aingle_union_datum_t *unionp;
			unionp = aingle_datum_to_union(datum);
			return (unionp->value == NULL)? 0:
			    aingle_datum_reset(unionp->value);
		}

		default:
			return 0;
	}
}

aingle_datum_t aingle_datum_incref(aingle_datum_t datum)
{
	if (datum) {
		aingle_refcount_inc(&datum->refcount);
	}
	return datum;
}

void aingle_datum_decref(aingle_datum_t datum)
{
	if (datum && aingle_refcount_dec(&datum->refcount)) {
		aingle_datum_free(datum);
	}
}

void aingle_datum_print(aingle_datum_t value, FILE * fp)
{
	AINGLE_UNUSED(value);
	AINGLE_UNUSED(fp);
}
