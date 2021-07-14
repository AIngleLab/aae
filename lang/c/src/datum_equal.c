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
#include <string.h>
#include "datum.h"

static int
array_equal(struct aingle_array_datum_t *a, struct aingle_array_datum_t *b)
{
	if (!aingle_schema_equal(a->schema, b->schema)) {
		return 0;
	}

	long i;

	if (a->els->num_entries != b->els->num_entries) {
		return 0;
	}
	for (i = 0; i < a->els->num_entries; i++) {
		union {
			st_data_t data;
			aingle_datum_t datum;
		} ael, bel;
		st_lookup(a->els, i, &ael.data);
		st_lookup(b->els, i, &bel.data);
		if (!aingle_datum_equal(ael.datum, bel.datum)) {
			return 0;
		}
	}
	return 1;
}

struct st_equal_args {
	int rval;
	st_table *st;
};

static int
st_equal_foreach(char *key, aingle_datum_t datum, struct st_equal_args *args)
{
	union {
		aingle_datum_t datum_other;
		st_data_t data;
	} val;
	if (!st_lookup(args->st, (st_data_t) key, &(val.data))) {
		args->rval = 0;
		return ST_STOP;
	}
	if (!aingle_datum_equal(datum, val.datum_other)) {
		args->rval = 0;
		return ST_STOP;
	}
	return ST_CONTINUE;
}

static int map_equal(struct aingle_map_datum_t *a, struct aingle_map_datum_t *b)
{
	if (!aingle_schema_equal(a->schema, b->schema)) {
		return 0;
	}

	struct st_equal_args args = { 1, b->map };
	if (a->map->num_entries != b->map->num_entries) {
		return 0;
	}
	st_foreach(a->map, HASH_FUNCTION_CAST st_equal_foreach, (st_data_t) & args);
	return args.rval;
}

static int record_equal(struct aingle_record_datum_t *a,
			struct aingle_record_datum_t *b)
{
	if (!aingle_schema_equal(a->schema, b->schema)) {
		return 0;
	}

	struct st_equal_args args = { 1, b->fields_byname };
	if (a->fields_byname->num_entries != b->fields_byname->num_entries) {
		return 0;
	}
	st_foreach(a->fields_byname, HASH_FUNCTION_CAST st_equal_foreach, (st_data_t) & args);
	return args.rval;
}

static int enum_equal(struct aingle_enum_datum_t *a, struct aingle_enum_datum_t *b)
{
	return aingle_schema_equal(a->schema, b->schema) && a->value == b->value;
}

static int fixed_equal(struct aingle_fixed_datum_t *a,
		       struct aingle_fixed_datum_t *b)
{
	if (!aingle_schema_equal(a->schema, b->schema)) {
		return 0;
	}

	return a->size == b->size && memcmp(a->bytes, b->bytes, a->size) == 0;
}

static int union_equal(struct aingle_union_datum_t *a,
		       struct aingle_union_datum_t *b)
{
	if (!aingle_schema_equal(a->schema, b->schema)) {
		return 0;
	}

	return a->discriminant == b->discriminant && aingle_datum_equal(a->value, b->value);
}

int aingle_datum_equal(const aingle_datum_t a, const aingle_datum_t b)
{
	if (!(is_aingle_datum(a) && is_aingle_datum(b))) {
		return 0;
	}
	if (aingle_typeof(a) != aingle_typeof(b)) {
		return 0;
	}
	switch (aingle_typeof(a)) {
	case AINGLE_STRING:
		return strcmp(aingle_datum_to_string(a)->s,
			      aingle_datum_to_string(b)->s) == 0;
	case AINGLE_BYTES:
		return (aingle_datum_to_bytes(a)->size ==
			aingle_datum_to_bytes(b)->size)
		    && memcmp(aingle_datum_to_bytes(a)->bytes,
			      aingle_datum_to_bytes(b)->bytes,
			      aingle_datum_to_bytes(a)->size) == 0;
	case AINGLE_INT32:
		return aingle_datum_to_int32(a)->i32 ==
		    aingle_datum_to_int32(b)->i32;
	case AINGLE_INT64:
		return aingle_datum_to_int64(a)->i64 ==
		    aingle_datum_to_int64(b)->i64;
	case AINGLE_FLOAT:
		return aingle_datum_to_float(a)->f == aingle_datum_to_float(b)->f;
	case AINGLE_DOUBLE:
		return aingle_datum_to_double(a)->d == aingle_datum_to_double(b)->d;
	case AINGLE_BOOLEAN:
		return aingle_datum_to_boolean(a)->i ==
		    aingle_datum_to_boolean(b)->i;
	case AINGLE_NULL:
		return 1;
	case AINGLE_ARRAY:
		return array_equal(aingle_datum_to_array(a),
				   aingle_datum_to_array(b));
	case AINGLE_MAP:
		return map_equal(aingle_datum_to_map(a), aingle_datum_to_map(b));

	case AINGLE_RECORD:
		return record_equal(aingle_datum_to_record(a),
				    aingle_datum_to_record(b));

	case AINGLE_ENUM:
		return enum_equal(aingle_datum_to_enum(a), aingle_datum_to_enum(b));

	case AINGLE_FIXED:
		return fixed_equal(aingle_datum_to_fixed(a),
				   aingle_datum_to_fixed(b));

	case AINGLE_UNION:
		return union_equal(aingle_datum_to_union(a),
				   aingle_datum_to_union(b));

	case AINGLE_LINK:
		/*
		 * TODO 
		 */
		return 0;
	}
	return 0;
}
