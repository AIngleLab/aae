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

#ifndef AINGLE_DATUM_H
#define AINGLE_DATUM_H
#include <aingle/platform.h>
#include "aingle/basics.h"
#include "aingle/data.h"
#include "aingle/legacy.h"
#include "aingle/schema.h"
#include "aingle_private.h"
#include "st.h"

struct aingle_string_datum_t {
	struct aingle_obj_t obj;
	char *s;
	int64_t size;
	aingle_free_func_t  free;
};

struct aingle_bytes_datum_t {
	struct aingle_obj_t obj;
	char *bytes;
	int64_t size;
	aingle_free_func_t  free;
};

struct aingle_int32_datum_t {
	struct aingle_obj_t obj;
	int32_t i32;
};

struct aingle_int64_datum_t {
	struct aingle_obj_t obj;
	int64_t i64;
};

struct aingle_float_datum_t {
	struct aingle_obj_t obj;
	float f;
};

struct aingle_double_datum_t {
	struct aingle_obj_t obj;
	double d;
};

struct aingle_boolean_datum_t {
	struct aingle_obj_t obj;
	int8_t i;
};

struct aingle_fixed_datum_t {
	struct aingle_obj_t obj;
	aingle_schema_t schema;
	char *bytes;
	int64_t size;
	aingle_free_func_t  free;
};

struct aingle_map_datum_t {
	struct aingle_obj_t obj;
	aingle_schema_t schema;
	st_table *map;
	st_table *indices_by_key;
	st_table *keys_by_index;
};

struct aingle_record_datum_t {
	struct aingle_obj_t obj;
	aingle_schema_t schema;
	st_table *field_order;
	st_table *fields_byname;
};

struct aingle_enum_datum_t {
	struct aingle_obj_t obj;
	aingle_schema_t schema;
	int value;
};

struct aingle_array_datum_t {
	struct aingle_obj_t obj;
	aingle_schema_t schema;
	st_table *els;
};

struct aingle_union_datum_t {
	struct aingle_obj_t obj;
	aingle_schema_t schema;
	int64_t discriminant;
	aingle_datum_t value;
};

#define aingle_datum_to_string(datum_)    (container_of(datum_, struct aingle_string_datum_t, obj))
#define aingle_datum_to_bytes(datum_)     (container_of(datum_, struct aingle_bytes_datum_t, obj))
#define aingle_datum_to_int32(datum_)     (container_of(datum_, struct aingle_int32_datum_t, obj))
#define aingle_datum_to_int64(datum_)     (container_of(datum_, struct aingle_int64_datum_t, obj))
#define aingle_datum_to_float(datum_)     (container_of(datum_, struct aingle_float_datum_t, obj))
#define aingle_datum_to_double(datum_)    (container_of(datum_, struct aingle_double_datum_t, obj))
#define aingle_datum_to_boolean(datum_)   (container_of(datum_, struct aingle_boolean_datum_t, obj))
#define aingle_datum_to_fixed(datum_)     (container_of(datum_, struct aingle_fixed_datum_t, obj))
#define aingle_datum_to_map(datum_)       (container_of(datum_, struct aingle_map_datum_t, obj))
#define aingle_datum_to_record(datum_)    (container_of(datum_, struct aingle_record_datum_t, obj))
#define aingle_datum_to_enum(datum_)      (container_of(datum_, struct aingle_enum_datum_t, obj))
#define aingle_datum_to_array(datum_)     (container_of(datum_, struct aingle_array_datum_t, obj))
#define aingle_datum_to_union(datum_)	(container_of(datum_, struct aingle_union_datum_t, obj))

#endif
