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

#ifndef AINGLE_SCHEMA_H
#define AINGLE_SCHEMA_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/platform.h>
#include <stdlib.h>

#include <aingle/basics.h>

typedef struct aingle_obj_t *aingle_schema_t;

aingle_schema_t aingle_schema_string(void);
aingle_schema_t aingle_schema_bytes(void);
aingle_schema_t aingle_schema_int(void);
aingle_schema_t aingle_schema_long(void);
aingle_schema_t aingle_schema_float(void);
aingle_schema_t aingle_schema_double(void);
aingle_schema_t aingle_schema_boolean(void);
aingle_schema_t aingle_schema_null(void);

aingle_schema_t aingle_schema_record(const char *name, const char *space);
aingle_schema_t aingle_schema_record_field_get(const aingle_schema_t
					   record, const char *field_name);
const char *aingle_schema_record_field_name(const aingle_schema_t schema, int index);
int aingle_schema_record_field_get_index(const aingle_schema_t schema,
				       const char *field_name);
aingle_schema_t aingle_schema_record_field_get_by_index
(const aingle_schema_t record, int index);
int aingle_schema_record_field_append(const aingle_schema_t record,
				    const char *field_name,
				    const aingle_schema_t type);
size_t aingle_schema_record_size(const aingle_schema_t record);

aingle_schema_t aingle_schema_enum(const char *name);
aingle_schema_t aingle_schema_enum_ns(const char *name, const char *space);
const char *aingle_schema_enum_get(const aingle_schema_t enump,
				 int index);
int aingle_schema_enum_get_by_name(const aingle_schema_t enump,
				 const char *symbol_name);
int aingle_schema_enum_symbol_append(const aingle_schema_t
				   enump, const char *symbol);
int aingle_schema_enum_number_of_symbols(const aingle_schema_t enump);

aingle_schema_t aingle_schema_fixed(const char *name, const int64_t len);
aingle_schema_t aingle_schema_fixed_ns(const char *name, const char *space,
				   const int64_t len);
int64_t aingle_schema_fixed_size(const aingle_schema_t fixed);

aingle_schema_t aingle_schema_map(const aingle_schema_t values);
aingle_schema_t aingle_schema_map_values(aingle_schema_t map);

aingle_schema_t aingle_schema_array(const aingle_schema_t items);
aingle_schema_t aingle_schema_array_items(aingle_schema_t array);

aingle_schema_t aingle_schema_union(void);
size_t aingle_schema_union_size(const aingle_schema_t union_schema);
int aingle_schema_union_append(const aingle_schema_t
			     union_schema, const aingle_schema_t schema);
aingle_schema_t aingle_schema_union_branch(aingle_schema_t union_schema,
				       int branch_index);
aingle_schema_t aingle_schema_union_branch_by_name
(aingle_schema_t union_schema, int *branch_index, const char *name);

aingle_schema_t aingle_schema_link(aingle_schema_t schema);
aingle_schema_t aingle_schema_link_target(aingle_schema_t schema);

typedef struct aingle_schema_error_t_ *aingle_schema_error_t;

int aingle_schema_from_json(const char *jsontext, int32_t unused1,
			  aingle_schema_t *schema, aingle_schema_error_t *unused2);

/* jsontext does not need to be NUL terminated.  length must *NOT*
 * include the NUL terminator, if one is present. */
int aingle_schema_from_json_length(const char *jsontext, size_t length,
				 aingle_schema_t *schema);

/* A helper macro for loading a schema from a string literal.  The
 * literal must be declared as a char[], not a char *, since we use the
 * sizeof operator to determine its length. */
#define aingle_schema_from_json_literal(json, schema) \
    (aingle_schema_from_json_length((json), sizeof((json))-1, (schema)))

int aingle_schema_to_specific(aingle_schema_t schema, const char *prefix);

aingle_schema_t aingle_schema_get_subschema(const aingle_schema_t schema,
         const char *name);
const char *aingle_schema_name(const aingle_schema_t schema);
const char *aingle_schema_namespace(const aingle_schema_t schema);
const char *aingle_schema_type_name(const aingle_schema_t schema);
aingle_schema_t aingle_schema_copy(aingle_schema_t schema);
int aingle_schema_equal(aingle_schema_t a, aingle_schema_t b);

aingle_schema_t aingle_schema_incref(aingle_schema_t schema);
int aingle_schema_decref(aingle_schema_t schema);

int aingle_schema_match(aingle_schema_t writers_schema,
		      aingle_schema_t readers_schema);

CLOSE_EXTERN
#endif
