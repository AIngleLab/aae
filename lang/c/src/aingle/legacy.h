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

#ifndef AINGLE_LEGACY_H
#define AINGLE_LEGACY_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/platform.h>
#include <stdio.h>

#include <aingle/basics.h>
#include <aingle/data.h>
#include <aingle/schema.h>
#include <aingle/value.h>

/*
 * This file defines the deprecated interface for handling AIngle values.
 * It's here solely for backwards compatibility.  New code should use
 * the aingle_value_t interface (defined in aingle/value.h).  The
 * aingle_datum_t type has been replaced by the “generic” implementation
 * of the value interface, which is defined in aingle/generic.h.  You can
 * also use your own application-specific types as AIngle values by
 * defining your own aingle_value_t implementation for them.
 */

/**
 * A function used to free a bytes, string, or fixed buffer once it is
 * no longer needed by the datum that wraps it.
 */

typedef void
(*aingle_free_func_t)(void *ptr, size_t sz);

/**
 * An aingle_free_func_t that frees the buffer using the custom allocator
 * provided to aingle_set_allocator.
 */

void
aingle_alloc_free_func(void *ptr, size_t sz);

/*
 * Datum constructors.  Each datum stores a reference to the schema that
 * the datum is an instance of.  The primitive datum constructors don't
 * need to take in an explicit aingle_schema_t parameter, since there's
 * only one schema that they could be an instance of.  The complex
 * constructors do need an explicit schema parameter.
 */

typedef struct aingle_obj_t *aingle_datum_t;
aingle_datum_t aingle_string(const char *str);
aingle_datum_t aingle_givestring(const char *str,
			     aingle_free_func_t free);
aingle_datum_t aingle_bytes(const char *buf, int64_t len);
aingle_datum_t aingle_givebytes(const char *buf, int64_t len,
			    aingle_free_func_t free);
aingle_datum_t aingle_int32(int32_t i);
aingle_datum_t aingle_int64(int64_t l);
aingle_datum_t aingle_float(float f);
aingle_datum_t aingle_double(double d);
aingle_datum_t aingle_boolean(int8_t i);
aingle_datum_t aingle_null(void);
aingle_datum_t aingle_record(aingle_schema_t schema);
aingle_datum_t aingle_enum(aingle_schema_t schema, int i);
aingle_datum_t aingle_fixed(aingle_schema_t schema,
			const char *bytes, const int64_t size);
aingle_datum_t aingle_givefixed(aingle_schema_t schema,
			    const char *bytes, const int64_t size,
			    aingle_free_func_t free);
aingle_datum_t aingle_map(aingle_schema_t schema);
aingle_datum_t aingle_array(aingle_schema_t schema);
aingle_datum_t aingle_union(aingle_schema_t schema,
			int64_t discriminant, const aingle_datum_t datum);

/**
 * Returns the schema that the datum is an instance of.
 */

aingle_schema_t aingle_datum_get_schema(const aingle_datum_t datum);

/*
 * Constructs a new aingle_datum_t instance that's appropriate for holding
 * values of the given schema.
 */

aingle_datum_t aingle_datum_from_schema(const aingle_schema_t schema);

/* getters */
int aingle_string_get(aingle_datum_t datum, char **p);
int aingle_bytes_get(aingle_datum_t datum, char **bytes, int64_t * size);
int aingle_int32_get(aingle_datum_t datum, int32_t * i);
int aingle_int64_get(aingle_datum_t datum, int64_t * l);
int aingle_float_get(aingle_datum_t datum, float *f);
int aingle_double_get(aingle_datum_t datum, double *d);
int aingle_boolean_get(aingle_datum_t datum, int8_t * i);

int aingle_enum_get(const aingle_datum_t datum);
const char *aingle_enum_get_name(const aingle_datum_t datum);
int aingle_fixed_get(aingle_datum_t datum, char **bytes, int64_t * size);
int aingle_record_get(const aingle_datum_t record, const char *field_name,
		    aingle_datum_t * value);

/*
 * A helper macro that extracts the value of the given field of a
 * record.
 */

#define aingle_record_get_field_value(rc, rec, typ, fname, ...)	\
	do {							\
		aingle_datum_t  field = NULL;			\
		(rc) = aingle_record_get((rec), (fname), &field);	\
		if (rc) break;					\
		(rc) = aingle_##typ##_get(field, __VA_ARGS__);	\
	} while (0)


int aingle_map_get(const aingle_datum_t datum, const char *key,
		 aingle_datum_t * value);
/*
 * For maps, the "index" for each entry is based on the order that they
 * were added to the map.
 */
int aingle_map_get_key(const aingle_datum_t datum, int index,
		     const char **key);
int aingle_map_get_index(const aingle_datum_t datum, const char *key,
		       int *index);
size_t aingle_map_size(const aingle_datum_t datum);
int aingle_array_get(const aingle_datum_t datum, int64_t index, aingle_datum_t * value);
size_t aingle_array_size(const aingle_datum_t datum);

/*
 * These accessors allow you to query the current branch of a union
 * value, returning either the branch's discriminant value or the
 * aingle_datum_t of the branch.  A union value can be uninitialized, in
 * which case the discriminant will be -1 and the datum NULL.
 */

int64_t aingle_union_discriminant(const aingle_datum_t datum);
aingle_datum_t aingle_union_current_branch(aingle_datum_t datum);

/* setters */
int aingle_string_set(aingle_datum_t datum, const char *p);
int aingle_givestring_set(aingle_datum_t datum, const char *p,
			aingle_free_func_t free);

int aingle_bytes_set(aingle_datum_t datum, const char *bytes, const int64_t size);
int aingle_givebytes_set(aingle_datum_t datum, const char *bytes,
		       const int64_t size,
		       aingle_free_func_t free);

int aingle_int32_set(aingle_datum_t datum, const int32_t i);
int aingle_int64_set(aingle_datum_t datum, const int64_t l);
int aingle_float_set(aingle_datum_t datum, const float f);
int aingle_double_set(aingle_datum_t datum, const double d);
int aingle_boolean_set(aingle_datum_t datum, const int8_t i);

int aingle_enum_set(aingle_datum_t datum, const int symbol_value);
int aingle_enum_set_name(aingle_datum_t datum, const char *symbol_name);
int aingle_fixed_set(aingle_datum_t datum, const char *bytes, const int64_t size);
int aingle_givefixed_set(aingle_datum_t datum, const char *bytes,
		       const int64_t size,
		       aingle_free_func_t free);

int aingle_record_set(aingle_datum_t record, const char *field_name,
		    aingle_datum_t value);

/*
 * A helper macro that sets the value of the given field of a record.
 */

#define aingle_record_set_field_value(rc, rec, typ, fname, ...)	\
	do {							\
		aingle_datum_t  field = NULL;			\
		(rc) = aingle_record_get((rec), (fname), &field);	\
		if (rc) break;					\
		(rc) = aingle_##typ##_set(field, __VA_ARGS__);	\
	} while (0)

int aingle_map_set(aingle_datum_t map, const char *key,
		 aingle_datum_t value);
int aingle_array_append_datum(aingle_datum_t array_datum,
			    aingle_datum_t datum);

/*
 * This function selects the active branch of a union value, and can be
 * safely called on an existing union to change the current branch.  If
 * the branch changes, we'll automatically construct a new aingle_datum_t
 * for the new branch's schema type.  If the desired branch is already
 * the active branch of the union, we'll leave the existing datum
 * instance as-is.  The branch datum will be placed into the "branch"
 * parameter, regardless of whether we have to create a new datum
 * instance or not.
 */

int aingle_union_set_discriminant(aingle_datum_t unionp,
				int discriminant,
				aingle_datum_t *branch);

/**
 * Resets a datum instance.  For arrays and maps, this frees all
 * elements and clears the container.  For records and unions, this
 * recursively resets any child datum instances.
 */

int
aingle_datum_reset(aingle_datum_t value);

/* reference counting */
aingle_datum_t aingle_datum_incref(aingle_datum_t value);
void aingle_datum_decref(aingle_datum_t value);

void aingle_datum_print(aingle_datum_t value, FILE * fp);

int aingle_datum_equal(aingle_datum_t a, aingle_datum_t b);

/*
 * Returns a string containing the JSON encoding of an AIngle value.  You
 * must free this string when you're done with it, using the standard
 * free() function.  (*Not* using the custom AIngle allocator.)
 */

int aingle_datum_to_json(const aingle_datum_t datum,
		       int one_line, char **json_str);


int aingle_schema_datum_validate(aingle_schema_t
			       expected_schema, aingle_datum_t datum);

/*
 * An aingle_value_t implementation for aingle_datum_t objects.
 */

aingle_value_iface_t *
aingle_datum_class(void);

/*
 * Creates a new aingle_value_t instance for the given datum.
 */

int
aingle_datum_as_value(aingle_value_t *value, aingle_datum_t src);


CLOSE_EXTERN
#endif
