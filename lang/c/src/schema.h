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
#ifndef AINGLE_SCHEMA_PRIV_H
#define AINGLE_SCHEMA_PRIV_H

#include <aingle/platform.h>
#include "aingle/basics.h"
#include "aingle/schema.h"
#include "aingle_private.h"
#include "st.h"

struct aingle_record_field_t {
	int index;
	char *name;
	aingle_schema_t type;
	/*
	 * TODO: default values 
	 */
};

struct aingle_record_schema_t {
	struct aingle_obj_t obj;
	char *name;
	char *space;
	st_table *fields;
	st_table *fields_byname;
};

struct aingle_enum_schema_t {
	struct aingle_obj_t obj;
	char *name;
	char *space;
	st_table *symbols;
	st_table *symbols_byname;
};

struct aingle_array_schema_t {
	struct aingle_obj_t obj;
	aingle_schema_t items;
};

struct aingle_map_schema_t {
	struct aingle_obj_t obj;
	aingle_schema_t values;
};

struct aingle_union_schema_t {
	struct aingle_obj_t obj;
	st_table *branches;
	st_table *branches_byname;
};

struct aingle_fixed_schema_t {
	struct aingle_obj_t obj;
	const char *name;
	const char *space;
	int64_t size;
};

struct aingle_link_schema_t {
	struct aingle_obj_t obj;
	aingle_schema_t to;
};

#define aingle_schema_to_record(schema_)  (container_of(schema_, struct aingle_record_schema_t, obj))
#define aingle_schema_to_enum(schema_)    (container_of(schema_, struct aingle_enum_schema_t, obj))
#define aingle_schema_to_array(schema_)   (container_of(schema_, struct aingle_array_schema_t, obj))
#define aingle_schema_to_map(schema_)     (container_of(schema_, struct aingle_map_schema_t, obj))
#define aingle_schema_to_union(schema_)   (container_of(schema_, struct aingle_union_schema_t, obj))
#define aingle_schema_to_fixed(schema_)   (container_of(schema_, struct aingle_fixed_schema_t, obj))
#define aingle_schema_to_link(schema_)    (container_of(schema_, struct aingle_link_schema_t, obj))

#endif
