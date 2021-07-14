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
#include "aingle/refcount.h"
#include "aingle/errors.h"
#include "aingle/io.h"
#include "aingle/legacy.h"
#include "aingle/schema.h"
#include "aingle_private.h"
#include <aingle/platform.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "jansson.h"
#include "st.h"
#include "schema.h"

#define DEFAULT_TABLE_SIZE 32

/* forward declaration */
static int
aingle_schema_to_json2(const aingle_schema_t schema, aingle_writer_t out,
		     const char *parent_namespace);

static void aingle_schema_init(aingle_schema_t schema, aingle_type_t type)
{
	schema->type = type;
	schema->class_type = AINGLE_SCHEMA;
	aingle_refcount_set(&schema->refcount, 1);
}

static int is_aingle_id(const char *name)
{
	size_t i, len;
	if (name) {
		len = strlen(name);
		if (len < 1) {
			return 0;
		}
		for (i = 0; i < len; i++) {
			if (!(isalpha(name[i])
			      || name[i] == '_' || (i && isdigit(name[i])))) {
				return 0;
			}
		}
		/*
		 * starts with [A-Za-z_] subsequent [A-Za-z0-9_]
		 */
		return 1;
	}
	return 0;
}

/* Splits a qualified name by the last period, e.g. fullname "foo.bar.Baz" into
 * name "Baz" and namespace "foo.bar". Sets name_out to the name part (pointing
 * to a later position in the buffer that was passed in), and returns the
 * namespace (as a newly allocated buffer using AIngle's allocator). */
static char *split_namespace_name(const char *fullname, const char **name_out)
{
	char *last_dot = strrchr(fullname, '.');
	if (last_dot == NULL) {
		*name_out = fullname;
		return NULL;
	} else {
		*name_out = last_dot + 1;
		return aingle_strndup(fullname, last_dot - fullname);
	}
}

static int record_free_foreach(int i, struct aingle_record_field_t *field,
			       void *arg)
{
	AINGLE_UNUSED(i);
	AINGLE_UNUSED(arg);

	aingle_str_free(field->name);
	aingle_schema_decref(field->type);
	aingle_freet(struct aingle_record_field_t, field);
	return ST_DELETE;
}

static int enum_free_foreach(int i, char *sym, void *arg)
{
	AINGLE_UNUSED(i);
	AINGLE_UNUSED(arg);

	aingle_str_free(sym);
	return ST_DELETE;
}

static int union_free_foreach(int i, aingle_schema_t schema, void *arg)
{
	AINGLE_UNUSED(i);
	AINGLE_UNUSED(arg);

	aingle_schema_decref(schema);
	return ST_DELETE;
}

static void aingle_schema_free(aingle_schema_t schema)
{
	if (is_aingle_schema(schema)) {
		switch (aingle_typeof(schema)) {
		case AINGLE_STRING:
		case AINGLE_BYTES:
		case AINGLE_INT32:
		case AINGLE_INT64:
		case AINGLE_FLOAT:
		case AINGLE_DOUBLE:
		case AINGLE_BOOLEAN:
		case AINGLE_NULL:
			/* no memory allocated for primitives */
			return;

		case AINGLE_RECORD:{
				struct aingle_record_schema_t *record;
				record = aingle_schema_to_record(schema);
				aingle_str_free(record->name);
				if (record->space) {
					aingle_str_free(record->space);
				}
				st_foreach(record->fields, HASH_FUNCTION_CAST record_free_foreach,
					   0);
				st_free_table(record->fields_byname);
				st_free_table(record->fields);
				aingle_freet(struct aingle_record_schema_t, record);
			}
			break;

		case AINGLE_ENUM:{
				struct aingle_enum_schema_t *enump;
				enump = aingle_schema_to_enum(schema);
				aingle_str_free(enump->name);
				if (enump->space) {
					aingle_str_free(enump->space);
				}
				st_foreach(enump->symbols, HASH_FUNCTION_CAST enum_free_foreach,
					   0);
				st_free_table(enump->symbols);
				st_free_table(enump->symbols_byname);
				aingle_freet(struct aingle_enum_schema_t, enump);
			}
			break;

		case AINGLE_FIXED:{
				struct aingle_fixed_schema_t *fixed;
				fixed = aingle_schema_to_fixed(schema);
				aingle_str_free((char *) fixed->name);
				if (fixed->space) {
					aingle_str_free((char *) fixed->space);
				}
				aingle_freet(struct aingle_fixed_schema_t, fixed);
			}
			break;

		case AINGLE_MAP:{
				struct aingle_map_schema_t *map;
				map = aingle_schema_to_map(schema);
				aingle_schema_decref(map->values);
				aingle_freet(struct aingle_map_schema_t, map);
			}
			break;

		case AINGLE_ARRAY:{
				struct aingle_array_schema_t *array;
				array = aingle_schema_to_array(schema);
				aingle_schema_decref(array->items);
				aingle_freet(struct aingle_array_schema_t, array);
			}
			break;
		case AINGLE_UNION:{
				struct aingle_union_schema_t *unionp;
				unionp = aingle_schema_to_union(schema);
				st_foreach(unionp->branches, HASH_FUNCTION_CAST union_free_foreach,
					   0);
				st_free_table(unionp->branches);
				st_free_table(unionp->branches_byname);
				aingle_freet(struct aingle_union_schema_t, unionp);
			}
			break;

		case AINGLE_LINK:{
				struct aingle_link_schema_t *link;
				link = aingle_schema_to_link(schema);
				/* Since we didn't increment the
				 * reference count of the target
				 * schema when we created the link, we
				 * should not decrement the reference
				 * count of the target schema when we
				 * free the link.
				 */
				aingle_freet(struct aingle_link_schema_t, link);
			}
			break;
		}
	}
}

aingle_schema_t aingle_schema_incref(aingle_schema_t schema)
{
	if (schema) {
		aingle_refcount_inc(&schema->refcount);
	}
	return schema;
}

int
aingle_schema_decref(aingle_schema_t schema)
{
	if (schema && aingle_refcount_dec(&schema->refcount)) {
		aingle_schema_free(schema);
		return 0;
	}
	return 1;
}

aingle_schema_t aingle_schema_string(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_STRING,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_bytes(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_BYTES,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_int(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_INT32,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_long(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_INT64,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_float(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_FLOAT,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_double(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_DOUBLE,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_boolean(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_BOOLEAN,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_null(void)
{
	static struct aingle_obj_t obj = {
		AINGLE_NULL,
		AINGLE_SCHEMA,
		1
	};
	return aingle_schema_incref(&obj);
}

aingle_schema_t aingle_schema_fixed(const char *name, const int64_t size)
{
	return aingle_schema_fixed_ns(name, NULL, size);
}

aingle_schema_t aingle_schema_fixed_ns(const char *name, const char *space,
		const int64_t size)
{
	if (!is_aingle_id(name)) {
		aingle_set_error("Invalid AIngle identifier");
		return NULL;
	}

	struct aingle_fixed_schema_t *fixed =
	    (struct aingle_fixed_schema_t *) aingle_new(struct aingle_fixed_schema_t);
	if (!fixed) {
		aingle_set_error("Cannot allocate new fixed schema");
		return NULL;
	}
	fixed->name = aingle_strdup(name);
	if (!fixed->name) {
		aingle_set_error("Cannot allocate new fixed schema");
		aingle_freet(struct aingle_fixed_schema_t, fixed);
		return NULL;
	}
	fixed->space = space ? aingle_strdup(space) : NULL;
	if (space && !fixed->space) {
		aingle_set_error("Cannot allocate new fixed schema");
		aingle_str_free((char *) fixed->name);
		aingle_freet(struct aingle_fixed_schema_t, fixed);
		return NULL;
	}
	fixed->size = size;
	aingle_schema_init(&fixed->obj, AINGLE_FIXED);
	return &fixed->obj;
}

int64_t aingle_schema_fixed_size(const aingle_schema_t fixed)
{
	return aingle_schema_to_fixed(fixed)->size;
}

aingle_schema_t aingle_schema_union(void)
{
	struct aingle_union_schema_t *schema =
	    (struct aingle_union_schema_t *) aingle_new(struct aingle_union_schema_t);
	if (!schema) {
		aingle_set_error("Cannot allocate new union schema");
		return NULL;
	}
	schema->branches = st_init_numtable_with_size(DEFAULT_TABLE_SIZE);
	if (!schema->branches) {
		aingle_set_error("Cannot allocate new union schema");
		aingle_freet(struct aingle_union_schema_t, schema);
		return NULL;
	}
	schema->branches_byname =
	    st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!schema->branches_byname) {
		aingle_set_error("Cannot allocate new union schema");
		st_free_table(schema->branches);
		aingle_freet(struct aingle_union_schema_t, schema);
		return NULL;
	}

	aingle_schema_init(&schema->obj, AINGLE_UNION);
	return &schema->obj;
}

int
aingle_schema_union_append(const aingle_schema_t union_schema,
			 const aingle_schema_t schema)
{
	check_param(EINVAL, is_aingle_schema(union_schema), "union schema");
	check_param(EINVAL, is_aingle_union(union_schema), "union schema");
	check_param(EINVAL, is_aingle_schema(schema), "schema");

	struct aingle_union_schema_t *unionp = aingle_schema_to_union(union_schema);
	int  new_index = unionp->branches->num_entries;
	st_insert(unionp->branches, new_index, (st_data_t) schema);
	const char *name = aingle_schema_type_name(schema);
	st_insert(unionp->branches_byname, (st_data_t) name,
		  (st_data_t) new_index);
	aingle_schema_incref(schema);
	return 0;
}

size_t aingle_schema_union_size(const aingle_schema_t union_schema)
{
	check_param(EINVAL, is_aingle_schema(union_schema), "union schema");
	check_param(EINVAL, is_aingle_union(union_schema), "union schema");
	struct aingle_union_schema_t *unionp = aingle_schema_to_union(union_schema);
	return unionp->branches->num_entries;
}

aingle_schema_t aingle_schema_union_branch(aingle_schema_t unionp,
				       int branch_index)
{
	union {
		st_data_t data;
		aingle_schema_t schema;
	} val;
	if (st_lookup(aingle_schema_to_union(unionp)->branches,
		      branch_index, &val.data)) {
		return val.schema;
	} else {
		aingle_set_error("No union branch for discriminant %d",
			       branch_index);
		return NULL;
	}
}

aingle_schema_t aingle_schema_union_branch_by_name
(aingle_schema_t unionp, int *branch_index, const char *name)
{
	union {
		st_data_t data;
		int  branch_index;
	} val;

	if (!st_lookup(aingle_schema_to_union(unionp)->branches_byname,
		       (st_data_t) name, &val.data)) {
		aingle_set_error("No union branch named %s", name);
		return NULL;
	}

	if (branch_index != NULL) {
		*branch_index = val.branch_index;
	}
	return aingle_schema_union_branch(unionp, val.branch_index);
}

aingle_schema_t aingle_schema_array(const aingle_schema_t items)
{
	struct aingle_array_schema_t *array =
	    (struct aingle_array_schema_t *) aingle_new(struct aingle_array_schema_t);
	if (!array) {
		aingle_set_error("Cannot allocate new array schema");
		return NULL;
	}
	array->items = aingle_schema_incref(items);
	aingle_schema_init(&array->obj, AINGLE_ARRAY);
	return &array->obj;
}

aingle_schema_t aingle_schema_array_items(aingle_schema_t array)
{
	return aingle_schema_to_array(array)->items;
}

aingle_schema_t aingle_schema_map(const aingle_schema_t values)
{
	struct aingle_map_schema_t *map =
	    (struct aingle_map_schema_t *) aingle_new(struct aingle_map_schema_t);
	if (!map) {
		aingle_set_error("Cannot allocate new map schema");
		return NULL;
	}
	map->values = aingle_schema_incref(values);
	aingle_schema_init(&map->obj, AINGLE_MAP);
	return &map->obj;
}

aingle_schema_t aingle_schema_map_values(aingle_schema_t map)
{
	return aingle_schema_to_map(map)->values;
}

aingle_schema_t aingle_schema_enum(const char *name)
{
	return aingle_schema_enum_ns(name, NULL);
}

aingle_schema_t aingle_schema_enum_ns(const char *name, const char *space)
{
	if (!is_aingle_id(name)) {
		aingle_set_error("Invalid AIngle identifier");
		return NULL;
	}

	struct aingle_enum_schema_t *enump = (struct aingle_enum_schema_t *) aingle_new(struct aingle_enum_schema_t);
	if (!enump) {
		aingle_set_error("Cannot allocate new enum schema");
		return NULL;
	}
	enump->name = aingle_strdup(name);
	if (!enump->name) {
		aingle_set_error("Cannot allocate new enum schema");
		aingle_freet(struct aingle_enum_schema_t, enump);
		return NULL;
	}
	enump->space = space ? aingle_strdup(space) : NULL;
	if (space && !enump->space) {
		aingle_set_error("Cannot allocate new enum schema");
		aingle_str_free(enump->name);
		aingle_freet(struct aingle_enum_schema_t, enump);
		return NULL;
	}
	enump->symbols = st_init_numtable_with_size(DEFAULT_TABLE_SIZE);
	if (!enump->symbols) {
		aingle_set_error("Cannot allocate new enum schema");
		if (enump->space) aingle_str_free(enump->space);
		aingle_str_free(enump->name);
		aingle_freet(struct aingle_enum_schema_t, enump);
		return NULL;
	}
	enump->symbols_byname = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!enump->symbols_byname) {
		aingle_set_error("Cannot allocate new enum schema");
		st_free_table(enump->symbols);
		if (enump->space) aingle_str_free(enump->space);
		aingle_str_free(enump->name);
		aingle_freet(struct aingle_enum_schema_t, enump);
		return NULL;
	}
	aingle_schema_init(&enump->obj, AINGLE_ENUM);
	return &enump->obj;
}

const char *aingle_schema_enum_get(const aingle_schema_t enump,
				 int index)
{
	union {
		st_data_t data;
		char *sym;
	} val;
	st_lookup(aingle_schema_to_enum(enump)->symbols, index, &val.data);
	return val.sym;
}

int aingle_schema_enum_get_by_name(const aingle_schema_t enump,
				 const char *symbol_name)
{
	union {
		st_data_t data;
		long idx;
	} val;

	if (st_lookup(aingle_schema_to_enum(enump)->symbols_byname,
		      (st_data_t) symbol_name, &val.data)) {
		return val.idx;
	} else {
		aingle_set_error("No enum symbol named %s", symbol_name);
		return -1;
	}
}

int
aingle_schema_enum_symbol_append(const aingle_schema_t enum_schema,
			       const char *symbol)
{
	check_param(EINVAL, is_aingle_schema(enum_schema), "enum schema");
	check_param(EINVAL, is_aingle_enum(enum_schema), "enum schema");
	check_param(EINVAL, symbol, "symbol");

	char *sym;
	long idx;
	struct aingle_enum_schema_t *enump = aingle_schema_to_enum(enum_schema);
	sym = aingle_strdup(symbol);
	if (!sym) {
		aingle_set_error("Cannot create copy of symbol name");
		return ENOMEM;
	}
	idx = enump->symbols->num_entries;
	st_insert(enump->symbols, (st_data_t) idx, (st_data_t) sym);
	st_insert(enump->symbols_byname, (st_data_t) sym, (st_data_t) idx);
	return 0;
}

int
aingle_schema_enum_number_of_symbols(const aingle_schema_t enum_schema)
{
	check_param(EINVAL, is_aingle_schema(enum_schema), "enum schema");
	check_param(EINVAL, is_aingle_enum(enum_schema), "enum schema");

	struct aingle_enum_schema_t *enump = aingle_schema_to_enum(enum_schema);
	return enump->symbols->num_entries;
}

int
aingle_schema_record_field_append(const aingle_schema_t record_schema,
				const char *field_name,
				const aingle_schema_t field_schema)
{
	check_param(EINVAL, is_aingle_schema(record_schema), "record schema");
	check_param(EINVAL, is_aingle_record(record_schema), "record schema");
	check_param(EINVAL, field_name, "field name");
	check_param(EINVAL, is_aingle_schema(field_schema), "field schema");

	if (!is_aingle_id(field_name)) {
		aingle_set_error("Invalid AIngle identifier");
		return EINVAL;
	}

	if (record_schema == field_schema) {
		aingle_set_error("Cannot create a circular schema");
		return EINVAL;
	}

	struct aingle_record_schema_t *record = aingle_schema_to_record(record_schema);
	struct aingle_record_field_t *new_field = (struct aingle_record_field_t *) aingle_new(struct aingle_record_field_t);
	if (!new_field) {
		aingle_set_error("Cannot allocate new record field");
		return ENOMEM;
	}
	new_field->index = record->fields->num_entries;
	new_field->name = aingle_strdup(field_name);
	new_field->type = aingle_schema_incref(field_schema);
	st_insert(record->fields, record->fields->num_entries,
		  (st_data_t) new_field);
	st_insert(record->fields_byname, (st_data_t) new_field->name,
		  (st_data_t) new_field);
	return 0;
}

aingle_schema_t aingle_schema_record(const char *name, const char *space)
{
	if (!is_aingle_id(name)) {
		aingle_set_error("Invalid AIngle identifier");
		return NULL;
	}

	struct aingle_record_schema_t *record = (struct aingle_record_schema_t *) aingle_new(struct aingle_record_schema_t);
	if (!record) {
		aingle_set_error("Cannot allocate new record schema");
		return NULL;
	}
	record->name = aingle_strdup(name);
	if (!record->name) {
		aingle_set_error("Cannot allocate new record schema");
		aingle_freet(struct aingle_record_schema_t, record);
		return NULL;
	}
	record->space = space ? aingle_strdup(space) : NULL;
	if (space && !record->space) {
		aingle_set_error("Cannot allocate new record schema");
		aingle_str_free(record->name);
		aingle_freet(struct aingle_record_schema_t, record);
		return NULL;
	}
	record->fields = st_init_numtable_with_size(DEFAULT_TABLE_SIZE);
	if (!record->fields) {
		aingle_set_error("Cannot allocate new record schema");
		if (record->space) {
			aingle_str_free(record->space);
		}
		aingle_str_free(record->name);
		aingle_freet(struct aingle_record_schema_t, record);
		return NULL;
	}
	record->fields_byname = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!record->fields_byname) {
		aingle_set_error("Cannot allocate new record schema");
		st_free_table(record->fields);
		if (record->space) {
			aingle_str_free(record->space);
		}
		aingle_str_free(record->name);
		aingle_freet(struct aingle_record_schema_t, record);
		return NULL;
	}

	aingle_schema_init(&record->obj, AINGLE_RECORD);
	return &record->obj;
}

size_t aingle_schema_record_size(const aingle_schema_t record)
{
	return aingle_schema_to_record(record)->fields->num_entries;
}

aingle_schema_t aingle_schema_record_field_get(const aingle_schema_t
					   record, const char *field_name)
{
	union {
		st_data_t data;
		struct aingle_record_field_t *field;
	} val;
	st_lookup(aingle_schema_to_record(record)->fields_byname,
		  (st_data_t) field_name, &val.data);
	return val.field->type;
}

int aingle_schema_record_field_get_index(const aingle_schema_t schema,
				       const char *field_name)
{
	union {
		st_data_t data;
		struct aingle_record_field_t *field;
	} val;
	if (st_lookup(aingle_schema_to_record(schema)->fields_byname,
		      (st_data_t) field_name, &val.data)) {
		return val.field->index;
	}

	aingle_set_error("No field named %s in record", field_name);
	return -1;
}

const char *aingle_schema_record_field_name(const aingle_schema_t schema, int index)
{
	union {
		st_data_t data;
		struct aingle_record_field_t *field;
	} val;
	st_lookup(aingle_schema_to_record(schema)->fields, index, &val.data);
	return val.field->name;
}

aingle_schema_t aingle_schema_record_field_get_by_index
(const aingle_schema_t record, int index)
{
	union {
		st_data_t data;
		struct aingle_record_field_t *field;
	} val;
	st_lookup(aingle_schema_to_record(record)->fields, index, &val.data);
	return val.field->type;
}

aingle_schema_t aingle_schema_link(aingle_schema_t to)
{
	if (!is_aingle_named_type(to)) {
		aingle_set_error("Can only link to named types");
		return NULL;
	}

	struct aingle_link_schema_t *link = (struct aingle_link_schema_t *) aingle_new(struct aingle_link_schema_t);
	if (!link) {
		aingle_set_error("Cannot allocate new link schema");
		return NULL;
	}

	/* Do not increment the reference count of target schema
	 * pointed to by the AINGLE_LINK. AINGLE_LINKs are only valid
	 * internal to a schema. The target schema pointed to by a
	 * link will be valid as long as the top-level schema is
	 * valid. Similarly, the link will be valid as long as the
	 * top-level schema is valid. Therefore the validity of the
	 * link ensures the validity of its target, and we don't need
	 * an additional reference count on the target. This mechanism
	 * of an implied validity also breaks reference count cycles
	 * for recursive schemas, which result in memory leaks.
	 */
	link->to = to;
	aingle_schema_init(&link->obj, AINGLE_LINK);
	return &link->obj;
}

aingle_schema_t aingle_schema_link_target(aingle_schema_t schema)
{
	check_param(NULL, is_aingle_schema(schema), "schema");
	check_param(NULL, is_aingle_link(schema), "schema");

	struct aingle_link_schema_t *link = aingle_schema_to_link(schema);
	return link->to;
}

static const char *
qualify_name(const char *name, const char *namespace)
{
	char *full_name;
	if (namespace != NULL && strchr(name, '.') == NULL) {
		full_name = aingle_str_alloc(strlen(name) + strlen(namespace) + 2);
		sprintf(full_name, "%s.%s", namespace, name);
	} else {
		full_name = aingle_strdup(name);
	}
	return full_name;
}

static int
save_named_schemas(const aingle_schema_t schema, st_table *st)
{
	const char *name = aingle_schema_name(schema);
	const char *namespace = aingle_schema_namespace(schema);
	const char *full_name = qualify_name(name, namespace);
	int rval = st_insert(st, (st_data_t) full_name, (st_data_t) schema);
	return rval;
}

static aingle_schema_t
find_named_schemas(const char *name, const char *namespace, st_table *st)
{
	union {
		aingle_schema_t schema;
		st_data_t data;
	} val;
	const char *full_name = qualify_name(name, namespace);
	int rval = st_lookup(st, (st_data_t) full_name, &(val.data));
	aingle_str_free((char *)full_name);
	if (rval) {
		return val.schema;
	}
	aingle_set_error("No schema type named %s", name);
	return NULL;
};

static int
aingle_type_from_json_t(json_t *json, aingle_type_t *type,
		      st_table *named_schemas, aingle_schema_t *named_type,
		      const char *namespace)
{
	json_t *json_type;
	const char *type_str;

	if (json_is_array(json)) {
		*type = AINGLE_UNION;
		return 0;
	} else if (json_is_object(json)) {
		json_type = json_object_get(json, "type");
	} else {
		json_type = json;
	}
	if (!json_is_string(json_type)) {
		aingle_set_error("\"type\" field must be a string");
		return EINVAL;
	}
	type_str = json_string_value(json_type);
	if (!type_str) {
		aingle_set_error("\"type\" field must be a string");
		return EINVAL;
	}
	/*
	 * TODO: gperf/re2c this
	 */
	if (strcmp(type_str, "string") == 0) {
		*type = AINGLE_STRING;
	} else if (strcmp(type_str, "bytes") == 0) {
		*type = AINGLE_BYTES;
	} else if (strcmp(type_str, "int") == 0) {
		*type = AINGLE_INT32;
	} else if (strcmp(type_str, "long") == 0) {
		*type = AINGLE_INT64;
	} else if (strcmp(type_str, "float") == 0) {
		*type = AINGLE_FLOAT;
	} else if (strcmp(type_str, "double") == 0) {
		*type = AINGLE_DOUBLE;
	} else if (strcmp(type_str, "boolean") == 0) {
		*type = AINGLE_BOOLEAN;
	} else if (strcmp(type_str, "null") == 0) {
		*type = AINGLE_NULL;
	} else if (strcmp(type_str, "record") == 0) {
		*type = AINGLE_RECORD;
	} else if (strcmp(type_str, "enum") == 0) {
		*type = AINGLE_ENUM;
	} else if (strcmp(type_str, "array") == 0) {
		*type = AINGLE_ARRAY;
	} else if (strcmp(type_str, "map") == 0) {
		*type = AINGLE_MAP;
	} else if (strcmp(type_str, "fixed") == 0) {
		*type = AINGLE_FIXED;
	} else if ((*named_type = find_named_schemas(type_str, namespace, named_schemas))) {
		*type = AINGLE_LINK;
	} else {
		aingle_set_error("Unknown AIngle \"type\": %s", type_str);
		return EINVAL;
	}
	return 0;
}

static int
aingle_schema_from_json_t(json_t *json, aingle_schema_t *schema,
			st_table *named_schemas, const char *parent_namespace)
{
#ifdef _WIN32
 #pragma message("#warning: Bug: '0' is not of type aingle_type_t.")
#else
 #warning "Bug: '0' is not of type aingle_type_t."
#endif
  /* We should really have an "AINGLE_INVALID" type in
   * aingle_type_t. Suppress warning below in which we set type to 0.
   */
	aingle_type_t type = (aingle_type_t) 0;
	unsigned int i;
	aingle_schema_t named_type = NULL;

	if (aingle_type_from_json_t(json, &type, named_schemas, &named_type, parent_namespace)) {
		return EINVAL;
	}

	switch (type) {
	case AINGLE_LINK:
		*schema = aingle_schema_link(named_type);
		break;

	case AINGLE_STRING:
		*schema = aingle_schema_string();
		break;

	case AINGLE_BYTES:
		*schema = aingle_schema_bytes();
		break;

	case AINGLE_INT32:
		*schema = aingle_schema_int();
		break;

	case AINGLE_INT64:
		*schema = aingle_schema_long();
		break;

	case AINGLE_FLOAT:
		*schema = aingle_schema_float();
		break;

	case AINGLE_DOUBLE:
		*schema = aingle_schema_double();
		break;

	case AINGLE_BOOLEAN:
		*schema = aingle_schema_boolean();
		break;

	case AINGLE_NULL:
		*schema = aingle_schema_null();
		break;

	case AINGLE_RECORD:
		{
			json_t *json_name = json_object_get(json, "name");
			json_t *json_namespace =
			    json_object_get(json, "namespace");
			json_t *json_fields = json_object_get(json, "fields");
			unsigned int num_fields;
			const char *fullname, *name;

			if (!json_is_string(json_name)) {
				aingle_set_error("Record type must have a \"name\"");
				return EINVAL;
			}
			if (!json_is_array(json_fields)) {
				aingle_set_error("Record type must have \"fields\"");
				return EINVAL;
			}
			num_fields = json_array_size(json_fields);
			fullname = json_string_value(json_name);
			if (!fullname) {
				aingle_set_error("Record type must have a \"name\"");
				return EINVAL;
			}

			if (strchr(fullname, '.')) {
				char *namespace = split_namespace_name(fullname, &name);
				*schema = aingle_schema_record(name, namespace);
				aingle_str_free(namespace);
			} else if (json_is_string(json_namespace)) {
				const char *namespace = json_string_value(json_namespace);
				if (strlen(namespace) == 0) {
					namespace = NULL;
				}
				*schema = aingle_schema_record(fullname, namespace);
			} else {
				*schema = aingle_schema_record(fullname, parent_namespace);
			}

			if (*schema == NULL) {
				return ENOMEM;
			}
			if (save_named_schemas(*schema, named_schemas)) {
				aingle_set_error("Cannot save record schema");
				return ENOMEM;
			}
			for (i = 0; i < num_fields; i++) {
				json_t *json_field =
				    json_array_get(json_fields, i);
				json_t *json_field_name;
				json_t *json_field_type;
				aingle_schema_t json_field_type_schema;
				int field_rval;

				if (!json_is_object(json_field)) {
					aingle_set_error("Record field %d must be an array", i);
					aingle_schema_decref(*schema);
					return EINVAL;
				}
				json_field_name =
				    json_object_get(json_field, "name");
				if (!json_field_name) {
					aingle_set_error("Record field %d must have a \"name\"", i);
					aingle_schema_decref(*schema);
					return EINVAL;
				}
				json_field_type =
				    json_object_get(json_field, "type");
				if (!json_field_type) {
					aingle_set_error("Record field %d must have a \"type\"", i);
					aingle_schema_decref(*schema);
					return EINVAL;
				}
				field_rval =
				    aingle_schema_from_json_t(json_field_type,
							    &json_field_type_schema,
							    named_schemas,
							    aingle_schema_namespace(*schema));
				if (field_rval) {
					aingle_schema_decref(*schema);
					return field_rval;
				}
				field_rval =
				    aingle_schema_record_field_append(*schema,
								    json_string_value
								    (json_field_name),
								    json_field_type_schema);
				aingle_schema_decref(json_field_type_schema);
				if (field_rval != 0) {
					aingle_schema_decref(*schema);
					return field_rval;
				}
			}
		}
		break;

	case AINGLE_ENUM:
		{
			json_t *json_name = json_object_get(json, "name");
			json_t *json_symbols = json_object_get(json, "symbols");
			json_t *json_namespace = json_object_get(json, "namespace");
			const char *fullname, *name;
			unsigned int num_symbols;

			if (!json_is_string(json_name)) {
				aingle_set_error("Enum type must have a \"name\"");
				return EINVAL;
			}
			if (!json_is_array(json_symbols)) {
				aingle_set_error("Enum type must have \"symbols\"");
				return EINVAL;
			}

			fullname = json_string_value(json_name);
			if (!fullname) {
				aingle_set_error("Enum type must have a \"name\"");
				return EINVAL;
			}
			num_symbols = json_array_size(json_symbols);
			if (num_symbols == 0) {
				aingle_set_error("Enum type must have at least one symbol");
				return EINVAL;
			}

			if (strchr(fullname, '.')) {
				char *namespace;
				namespace = split_namespace_name(fullname, &name);
				*schema = aingle_schema_enum_ns(name, namespace);
				aingle_str_free(namespace);
			} else if (json_is_string(json_namespace)) {
				const char *namespace = json_string_value(json_namespace);
				if (strlen(namespace) == 0) {
					namespace = NULL;
				}
				*schema = aingle_schema_enum_ns(fullname, namespace);
			} else {
				*schema = aingle_schema_enum_ns(fullname, parent_namespace);
			}

			if (*schema == NULL) {
				return ENOMEM;
			}
			if (save_named_schemas(*schema, named_schemas)) {
				aingle_set_error("Cannot save enum schema");
				return ENOMEM;
			}
			for (i = 0; i < num_symbols; i++) {
				int enum_rval;
				json_t *json_symbol =
				    json_array_get(json_symbols, i);
				const char *symbol;
				if (!json_is_string(json_symbol)) {
					aingle_set_error("Enum symbol %d must be a string", i);
					aingle_schema_decref(*schema);
					return EINVAL;
				}
				symbol = json_string_value(json_symbol);
				enum_rval =
				    aingle_schema_enum_symbol_append(*schema,
								   symbol);
				if (enum_rval != 0) {
					aingle_schema_decref(*schema);
					return enum_rval;
				}
			}
		}
		break;

	case AINGLE_ARRAY:
		{
			int items_rval;
			json_t *json_items = json_object_get(json, "items");
			aingle_schema_t items_schema;
			if (!json_items) {
				aingle_set_error("Array type must have \"items\"");
				return EINVAL;
			}
			items_rval =
			    aingle_schema_from_json_t(json_items, &items_schema,
						    named_schemas, parent_namespace);
			if (items_rval) {
				return items_rval;
			}
			*schema = aingle_schema_array(items_schema);
			aingle_schema_decref(items_schema);
		}
		break;

	case AINGLE_MAP:
		{
			int values_rval;
			json_t *json_values = json_object_get(json, "values");
			aingle_schema_t values_schema;

			if (!json_values) {
				aingle_set_error("Map type must have \"values\"");
				return EINVAL;
			}
			values_rval =
			    aingle_schema_from_json_t(json_values, &values_schema,
						    named_schemas, parent_namespace);
			if (values_rval) {
				return values_rval;
			}
			*schema = aingle_schema_map(values_schema);
			aingle_schema_decref(values_schema);
		}
		break;

	case AINGLE_UNION:
		{
			unsigned int num_schemas = json_array_size(json);
			aingle_schema_t s;
			if (num_schemas == 0) {
				aingle_set_error("Union type must have at least one branch");
				return EINVAL;
			}
			*schema = aingle_schema_union();
			for (i = 0; i < num_schemas; i++) {
				int schema_rval;
				json_t *schema_json = json_array_get(json, i);
				if (!schema_json) {
					aingle_set_error("Cannot retrieve branch JSON");
					return EINVAL;
				}
				schema_rval =
				    aingle_schema_from_json_t(schema_json, &s,
							    named_schemas, parent_namespace);
				if (schema_rval != 0) {
					aingle_schema_decref(*schema);
					return schema_rval;
				}
				schema_rval =
				    aingle_schema_union_append(*schema, s);
				aingle_schema_decref(s);
				if (schema_rval != 0) {
					aingle_schema_decref(*schema);
					return schema_rval;
				}
			}
		}
		break;

	case AINGLE_FIXED:
		{
			json_t *json_size = json_object_get(json, "size");
			json_t *json_name = json_object_get(json, "name");
			json_t *json_namespace = json_object_get(json, "namespace");
			json_int_t size;
			const char *fullname, *name;
			if (!json_is_integer(json_size)) {
				aingle_set_error("Fixed type must have a \"size\"");
				return EINVAL;
			}
			if (!json_is_string(json_name)) {
				aingle_set_error("Fixed type must have a \"name\"");
				return EINVAL;
			}
			size = json_integer_value(json_size);
			fullname = json_string_value(json_name);

			if (strchr(fullname, '.')) {
				char *namespace;
				namespace = split_namespace_name(fullname, &name);
				*schema = aingle_schema_fixed_ns(name, namespace, (int64_t) size);
				aingle_str_free(namespace);
			} else if (json_is_string(json_namespace)) {
				const char *namespace = json_string_value(json_namespace);
				if (strlen(namespace) == 0) {
					namespace = NULL;
				}
				*schema = aingle_schema_fixed_ns(fullname, namespace, (int64_t) size);
			} else {
				*schema = aingle_schema_fixed_ns(fullname, parent_namespace, (int64_t) size);
			}

			if (*schema == NULL) {
				return ENOMEM;
			}
			if (save_named_schemas(*schema, named_schemas)) {
				aingle_set_error("Cannot save fixed schema");
				return ENOMEM;
			}
		}
		break;

	default:
		aingle_set_error("Unknown schema type");
		return EINVAL;
	}
	return 0;
}

static int named_schema_free_foreach(char *full_name, st_data_t value, st_data_t arg)
{
	AINGLE_UNUSED(value);
	AINGLE_UNUSED(arg);

	aingle_str_free(full_name);
	return ST_DELETE;
}

static int
aingle_schema_from_json_root(json_t *root, aingle_schema_t *schema)
{
	int  rval;
	st_table *named_schemas;

	named_schemas = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!named_schemas) {
		aingle_set_error("Cannot allocate named schema map");
		json_decref(root);
		return ENOMEM;
	}

	/* json_dumpf(root, stderr, 0); */
	rval = aingle_schema_from_json_t(root, schema, named_schemas, NULL);
	json_decref(root);
	st_foreach(named_schemas, HASH_FUNCTION_CAST named_schema_free_foreach, 0);
	st_free_table(named_schemas);
	return rval;
}

int
aingle_schema_from_json(const char *jsontext, const int32_t len,
		      aingle_schema_t *schema, aingle_schema_error_t *e)
{
	check_param(EINVAL, jsontext, "JSON text");
	check_param(EINVAL, schema, "schema pointer");

	json_t  *root;
	json_error_t  json_error;

	AINGLE_UNUSED(len);
	AINGLE_UNUSED(e);

	root = json_loads(jsontext, JSON_DECODE_ANY, &json_error);
	if (!root) {
		aingle_set_error("Error parsing JSON: %s", json_error.text);
		return EINVAL;
	}

	return aingle_schema_from_json_root(root, schema);
}

int
aingle_schema_from_json_length(const char *jsontext, size_t length,
			     aingle_schema_t *schema)
{
	check_param(EINVAL, jsontext, "JSON text");
	check_param(EINVAL, schema, "schema pointer");

	json_t  *root;
	json_error_t  json_error;

	root = json_loadb(jsontext, length, JSON_DECODE_ANY, &json_error);
	if (!root) {
		aingle_set_error("Error parsing JSON: %s", json_error.text);
		return EINVAL;
	}

	return aingle_schema_from_json_root(root, schema);
}

aingle_schema_t aingle_schema_copy_root(aingle_schema_t schema, st_table *named_schemas)
{
	long i;
	aingle_schema_t new_schema = NULL;
	if (!schema) {
		return NULL;
	}
	switch (aingle_typeof(schema)) {
	case AINGLE_STRING:
	case AINGLE_BYTES:
	case AINGLE_INT32:
	case AINGLE_INT64:
	case AINGLE_FLOAT:
	case AINGLE_DOUBLE:
	case AINGLE_BOOLEAN:
	case AINGLE_NULL:
		/*
		 * No need to copy primitives since they're static
		 */
		new_schema = schema;
		break;

	case AINGLE_RECORD:
		{
			struct aingle_record_schema_t *record_schema =
			    aingle_schema_to_record(schema);
			new_schema =
			    aingle_schema_record(record_schema->name,
					       record_schema->space);
		    if (save_named_schemas(new_schema, named_schemas)) {
   				aingle_set_error("Cannot save enum schema");
   				return NULL;
   			}
			for (i = 0; i < record_schema->fields->num_entries; i++) {
				union {
					st_data_t data;
					struct aingle_record_field_t *field;
				} val;
				st_lookup(record_schema->fields, i, &val.data);
				aingle_schema_t type_copy =
				    aingle_schema_copy_root(val.field->type, named_schemas);
				aingle_schema_record_field_append(new_schema,
								val.field->name,
								type_copy);
				aingle_schema_decref(type_copy);
			}
		}
		break;

	case AINGLE_ENUM:
		{
			struct aingle_enum_schema_t *enum_schema =
			    aingle_schema_to_enum(schema);
			new_schema = aingle_schema_enum_ns(enum_schema->name,
					enum_schema->space);
			if (save_named_schemas(new_schema, named_schemas)) {
				aingle_set_error("Cannot save enum schema");
				return NULL;
			}
			for (i = 0; i < enum_schema->symbols->num_entries; i++) {
				union {
					st_data_t data;
					char *sym;
				} val;
				st_lookup(enum_schema->symbols, i, &val.data);
				aingle_schema_enum_symbol_append(new_schema,
							       val.sym);
			}
		}
		break;

	case AINGLE_FIXED:
		{
			struct aingle_fixed_schema_t *fixed_schema =
			    aingle_schema_to_fixed(schema);
			new_schema =
			    aingle_schema_fixed_ns(fixed_schema->name,
					         fixed_schema->space,
					         fixed_schema->size);
 			if (save_named_schemas(new_schema, named_schemas)) {
 				aingle_set_error("Cannot save fixed schema");
 				return NULL;
 			}
		}
		break;

	case AINGLE_MAP:
		{
			struct aingle_map_schema_t *map_schema =
			    aingle_schema_to_map(schema);
			aingle_schema_t values_copy =
			    aingle_schema_copy_root(map_schema->values, named_schemas);
			if (!values_copy) {
				return NULL;
			}
			new_schema = aingle_schema_map(values_copy);
			aingle_schema_decref(values_copy);
		}
		break;

	case AINGLE_ARRAY:
		{
			struct aingle_array_schema_t *array_schema =
			    aingle_schema_to_array(schema);
			aingle_schema_t items_copy =
			    aingle_schema_copy_root(array_schema->items, named_schemas);
			if (!items_copy) {
				return NULL;
			}
			new_schema = aingle_schema_array(items_copy);
			aingle_schema_decref(items_copy);
		}
		break;

	case AINGLE_UNION:
		{
			struct aingle_union_schema_t *union_schema =
			    aingle_schema_to_union(schema);

			new_schema = aingle_schema_union();
			for (i = 0; i < union_schema->branches->num_entries;
			     i++) {
				aingle_schema_t schema_copy;
				union {
					st_data_t data;
					aingle_schema_t schema;
				} val;
				st_lookup(union_schema->branches, i, &val.data);
				schema_copy = aingle_schema_copy_root(val.schema, named_schemas);
				if (aingle_schema_union_append
				    (new_schema, schema_copy)) {
					aingle_schema_decref(new_schema);
					return NULL;
				}
				aingle_schema_decref(schema_copy);
			}
		}
		break;

	case AINGLE_LINK:
		{
			struct aingle_link_schema_t *link_schema =
			    aingle_schema_to_link(schema);
			aingle_schema_t to;

			to = find_named_schemas(aingle_schema_name(link_schema->to),
									aingle_schema_namespace(link_schema->to),
									named_schemas);
			new_schema = aingle_schema_link(to);
		}
		break;

	default:
		return NULL;
	}
	return new_schema;
}

aingle_schema_t aingle_schema_copy(aingle_schema_t schema)
{
	aingle_schema_t new_schema;
	st_table *named_schemas;

	named_schemas = st_init_strtable_with_size(DEFAULT_TABLE_SIZE);
	if (!named_schemas) {
		aingle_set_error("Cannot allocate named schema map");
		return NULL;
	}

	new_schema = aingle_schema_copy_root(schema, named_schemas);
	st_foreach(named_schemas, HASH_FUNCTION_CAST named_schema_free_foreach, 0);
	st_free_table(named_schemas);
	return new_schema;
}

aingle_schema_t aingle_schema_get_subschema(const aingle_schema_t schema,
         const char *name)
{
 if (is_aingle_record(schema)) {
   const struct aingle_record_schema_t *rschema =
     aingle_schema_to_record(schema);
   union {
     st_data_t data;
     struct aingle_record_field_t *field;
   } field;

   if (st_lookup(rschema->fields_byname,
           (st_data_t) name, &field.data))
   {
     return field.field->type;
   }

   aingle_set_error("No record field named %s", name);
   return NULL;
 } else if (is_aingle_union(schema)) {
   const struct aingle_union_schema_t *uschema =
     aingle_schema_to_union(schema);
   long i;

   for (i = 0; i < uschema->branches->num_entries; i++) {
     union {
       st_data_t data;
       aingle_schema_t schema;
     } val;
     st_lookup(uschema->branches, i, &val.data);
     if (strcmp(aingle_schema_type_name(val.schema),
          name) == 0)
     {
       return val.schema;
     }
   }

   aingle_set_error("No union branch named %s", name);
   return NULL;
 } else if (is_aingle_array(schema)) {
   if (strcmp(name, "[]") == 0) {
     const struct aingle_array_schema_t *aschema =
       aingle_schema_to_array(schema);
     return aschema->items;
   }

   aingle_set_error("Array subschema must be called \"[]\"");
   return NULL;
 } else if (is_aingle_map(schema)) {
   if (strcmp(name, "{}") == 0) {
     const struct aingle_map_schema_t *mschema =
       aingle_schema_to_map(schema);
     return mschema->values;
   }

   aingle_set_error("Map subschema must be called \"{}\"");
   return NULL;
 }

 aingle_set_error("Can only retrieve subschemas from record, union, array, or map");
 return NULL;
}

const char *aingle_schema_name(const aingle_schema_t schema)
{
	if (is_aingle_record(schema)) {
		return (aingle_schema_to_record(schema))->name;
	} else if (is_aingle_enum(schema)) {
		return (aingle_schema_to_enum(schema))->name;
	} else if (is_aingle_fixed(schema)) {
		return (aingle_schema_to_fixed(schema))->name;
	}
	aingle_set_error("Schema has no name");
	return NULL;
}

const char *aingle_schema_namespace(const aingle_schema_t schema)
{
	if (is_aingle_record(schema)) {
		return (aingle_schema_to_record(schema))->space;
	} else if (is_aingle_enum(schema)) {
		return (aingle_schema_to_enum(schema))->space;
	} else if (is_aingle_fixed(schema)) {
		return (aingle_schema_to_fixed(schema))->space;
	}
	return NULL;
}

const char *aingle_schema_type_name(const aingle_schema_t schema)
{
	if (is_aingle_record(schema)) {
		return (aingle_schema_to_record(schema))->name;
	} else if (is_aingle_enum(schema)) {
		return (aingle_schema_to_enum(schema))->name;
	} else if (is_aingle_fixed(schema)) {
		return (aingle_schema_to_fixed(schema))->name;
	} else if (is_aingle_union(schema)) {
		return "union";
	} else if (is_aingle_array(schema)) {
		return "array";
	} else if (is_aingle_map(schema)) {
		return "map";
	} else if (is_aingle_int32(schema)) {
		return "int";
	} else if (is_aingle_int64(schema)) {
		return "long";
	} else if (is_aingle_float(schema)) {
		return "float";
	} else if (is_aingle_double(schema)) {
		return "double";
	} else if (is_aingle_boolean(schema)) {
		return "boolean";
	} else if (is_aingle_null(schema)) {
		return "null";
	} else if (is_aingle_string(schema)) {
		return "string";
	} else if (is_aingle_bytes(schema)) {
		return "bytes";
	} else if (is_aingle_link(schema)) {
		aingle_schema_t  target = aingle_schema_link_target(schema);
		return aingle_schema_type_name(target);
	}
	aingle_set_error("Unknown schema type");
	return NULL;
}

aingle_datum_t aingle_datum_from_schema(const aingle_schema_t schema)
{
	check_param(NULL, is_aingle_schema(schema), "schema");

	switch (aingle_typeof(schema)) {
		case AINGLE_STRING:
			return aingle_givestring("", NULL);

		case AINGLE_BYTES:
			return aingle_givebytes("", 0, NULL);

		case AINGLE_INT32:
			return aingle_int32(0);

		case AINGLE_INT64:
			return aingle_int64(0);

		case AINGLE_FLOAT:
			return aingle_float(0);

		case AINGLE_DOUBLE:
			return aingle_double(0);

		case AINGLE_BOOLEAN:
			return aingle_boolean(0);

		case AINGLE_NULL:
			return aingle_null();

		case AINGLE_RECORD:
			{
				const struct aingle_record_schema_t *record_schema =
				    aingle_schema_to_record(schema);

				aingle_datum_t  rec = aingle_record(schema);

				int  i;
				for (i = 0; i < record_schema->fields->num_entries; i++) {
					union {
						st_data_t data;
						struct aingle_record_field_t *field;
					} val;
					st_lookup(record_schema->fields, i, &val.data);

					aingle_datum_t  field =
					    aingle_datum_from_schema(val.field->type);
					aingle_record_set(rec, val.field->name, field);
					aingle_datum_decref(field);
				}

				return rec;
			}

		case AINGLE_ENUM:
			return aingle_enum(schema, 0);

		case AINGLE_FIXED:
			{
				const struct aingle_fixed_schema_t *fixed_schema =
				    aingle_schema_to_fixed(schema);
				return aingle_givefixed(schema, NULL, fixed_schema->size, NULL);
			}

		case AINGLE_MAP:
			return aingle_map(schema);

		case AINGLE_ARRAY:
			return aingle_array(schema);

		case AINGLE_UNION:
			return aingle_union(schema, -1, NULL);

		case AINGLE_LINK:
			{
				const struct aingle_link_schema_t *link_schema =
				    aingle_schema_to_link(schema);
				return aingle_datum_from_schema(link_schema->to);
			}

		default:
			aingle_set_error("Unknown schema type");
			return NULL;
	}
}

/* simple helper for writing strings */
static int aingle_write_str(aingle_writer_t out, const char *str)
{
	return aingle_write(out, (char *)str, strlen(str));
}

static int write_field(aingle_writer_t out, const struct aingle_record_field_t *field,
		       const char *parent_namespace)
{
	int rval;
	check(rval, aingle_write_str(out, "{\"name\":\""));
	check(rval, aingle_write_str(out, field->name));
	check(rval, aingle_write_str(out, "\",\"type\":"));
	check(rval, aingle_schema_to_json2(field->type, out, parent_namespace));
	return aingle_write_str(out, "}");
}

static int write_record(aingle_writer_t out, const struct aingle_record_schema_t *record,
			const char *parent_namespace)
{
	int rval;
	long i;

	check(rval, aingle_write_str(out, "{\"type\":\"record\",\"name\":\""));
	check(rval, aingle_write_str(out, record->name));
	check(rval, aingle_write_str(out, "\","));
	if (nullstrcmp(record->space, parent_namespace)) {
		check(rval, aingle_write_str(out, "\"namespace\":\""));
		if (record->space) {
			check(rval, aingle_write_str(out, record->space));
		}
		check(rval, aingle_write_str(out, "\","));
	}
	check(rval, aingle_write_str(out, "\"fields\":["));
	for (i = 0; i < record->fields->num_entries; i++) {
		union {
			st_data_t data;
			struct aingle_record_field_t *field;
		} val;
		st_lookup(record->fields, i, &val.data);
		if (i) {
			check(rval, aingle_write_str(out, ","));
		}
		check(rval, write_field(out, val.field, record->space));
	}
	return aingle_write_str(out, "]}");
}

static int write_enum(aingle_writer_t out, const struct aingle_enum_schema_t *enump,
			const char *parent_namespace)
{
	int rval;
	long i;
	check(rval, aingle_write_str(out, "{\"type\":\"enum\",\"name\":\""));
	check(rval, aingle_write_str(out, enump->name));
	check(rval, aingle_write_str(out, "\","));
	if (nullstrcmp(enump->space, parent_namespace)) {
		check(rval, aingle_write_str(out, "\"namespace\":\""));
		if (enump->space) {
			check(rval, aingle_write_str(out, enump->space));
		}
		check(rval, aingle_write_str(out, "\","));
	}
	check(rval, aingle_write_str(out, "\"symbols\":["));

	for (i = 0; i < enump->symbols->num_entries; i++) {
		union {
			st_data_t data;
			char *sym;
		} val;
		st_lookup(enump->symbols, i, &val.data);
		if (i) {
			check(rval, aingle_write_str(out, ","));
		}
		check(rval, aingle_write_str(out, "\""));
		check(rval, aingle_write_str(out, val.sym));
		check(rval, aingle_write_str(out, "\""));
	}
	return aingle_write_str(out, "]}");
}

static int write_fixed(aingle_writer_t out, const struct aingle_fixed_schema_t *fixed,
			const char *parent_namespace)
{
	int rval;
	char size[16];
	check(rval, aingle_write_str(out, "{\"type\":\"fixed\",\"name\":\""));
	check(rval, aingle_write_str(out, fixed->name));
	check(rval, aingle_write_str(out, "\","));
	if (nullstrcmp(fixed->space, parent_namespace)) {
		check(rval, aingle_write_str(out, "\"namespace\":\""));
		if (fixed->space) {
			check(rval, aingle_write_str(out, fixed->space));
		}
		check(rval, aingle_write_str(out, "\","));
	}
	check(rval, aingle_write_str(out, "\"size\":"));
	snprintf(size, sizeof(size), "%" PRId64, fixed->size);
	check(rval, aingle_write_str(out, size));
	return aingle_write_str(out, "}");
}

static int write_map(aingle_writer_t out, const struct aingle_map_schema_t *map,
		     const char *parent_namespace)
{
	int rval;
	check(rval, aingle_write_str(out, "{\"type\":\"map\",\"values\":"));
	check(rval, aingle_schema_to_json2(map->values, out, parent_namespace));
	return aingle_write_str(out, "}");
}
static int write_array(aingle_writer_t out, const struct aingle_array_schema_t *array,
		       const char *parent_namespace)
{
	int rval;
	check(rval, aingle_write_str(out, "{\"type\":\"array\",\"items\":"));
	check(rval, aingle_schema_to_json2(array->items, out, parent_namespace));
	return aingle_write_str(out, "}");
}
static int write_union(aingle_writer_t out, const struct aingle_union_schema_t *unionp,
		       const char *parent_namespace)
{
	int rval;
	long i;
	check(rval, aingle_write_str(out, "["));

	for (i = 0; i < unionp->branches->num_entries; i++) {
		union {
			st_data_t data;
			aingle_schema_t schema;
		} val;
		st_lookup(unionp->branches, i, &val.data);
		if (i) {
			check(rval, aingle_write_str(out, ","));
		}
		check(rval, aingle_schema_to_json2(val.schema, out, parent_namespace));
	}
	return aingle_write_str(out, "]");
}
static int write_link(aingle_writer_t out, const struct aingle_link_schema_t *link,
		      const char *parent_namespace)
{
	int rval;
	check(rval, aingle_write_str(out, "\""));
	const char *namespace = aingle_schema_namespace(link->to);
	if (namespace && nullstrcmp(namespace, parent_namespace)) {
		check(rval, aingle_write_str(out, namespace));
		check(rval, aingle_write_str(out, "."));
	}
	check(rval, aingle_write_str(out, aingle_schema_name(link->to)));
	return aingle_write_str(out, "\"");
}

static int
aingle_schema_to_json2(const aingle_schema_t schema, aingle_writer_t out,
		     const char *parent_namespace)
{
	check_param(EINVAL, is_aingle_schema(schema), "schema");
	check_param(EINVAL, out, "writer");

	int rval;

	if (is_aingle_primitive(schema)) {
		check(rval, aingle_write_str(out, "{\"type\":\""));
	}

	switch (aingle_typeof(schema)) {
	case AINGLE_STRING:
		check(rval, aingle_write_str(out, "string"));
		break;
	case AINGLE_BYTES:
		check(rval, aingle_write_str(out, "bytes"));
		break;
	case AINGLE_INT32:
		check(rval, aingle_write_str(out, "int"));
		break;
	case AINGLE_INT64:
		check(rval, aingle_write_str(out, "long"));
		break;
	case AINGLE_FLOAT:
		check(rval, aingle_write_str(out, "float"));
		break;
	case AINGLE_DOUBLE:
		check(rval, aingle_write_str(out, "double"));
		break;
	case AINGLE_BOOLEAN:
		check(rval, aingle_write_str(out, "boolean"));
		break;
	case AINGLE_NULL:
		check(rval, aingle_write_str(out, "null"));
		break;
	case AINGLE_RECORD:
		return write_record(out, aingle_schema_to_record(schema), parent_namespace);
	case AINGLE_ENUM:
		return write_enum(out, aingle_schema_to_enum(schema), parent_namespace);
	case AINGLE_FIXED:
		return write_fixed(out, aingle_schema_to_fixed(schema), parent_namespace);
	case AINGLE_MAP:
		return write_map(out, aingle_schema_to_map(schema), parent_namespace);
	case AINGLE_ARRAY:
		return write_array(out, aingle_schema_to_array(schema), parent_namespace);
	case AINGLE_UNION:
		return write_union(out, aingle_schema_to_union(schema), parent_namespace);
	case AINGLE_LINK:
		return write_link(out, aingle_schema_to_link(schema), parent_namespace);
	}

	if (is_aingle_primitive(schema)) {
		return aingle_write_str(out, "\"}");
	}
	aingle_set_error("Unknown schema type");
	return EINVAL;
}

int aingle_schema_to_json(const aingle_schema_t schema, aingle_writer_t out)
{
	return aingle_schema_to_json2(schema, out, NULL);
}
