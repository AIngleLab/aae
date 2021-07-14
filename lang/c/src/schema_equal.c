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
#include "schema.h"
#include <string.h>

static int
schema_record_equal(struct aingle_record_schema_t *a,
		    struct aingle_record_schema_t *b)
{
	long i;
	if (strcmp(a->name, b->name)) {
		/*
		 * They have different names 
		 */
		return 0;
	}
	if (nullstrcmp(a->space, b->space)) {
		return 0;
	}
	if (a->fields->num_entries != b->fields->num_entries) {
		/* They have different numbers of fields */
		return 0;
	}
	for (i = 0; i < a->fields->num_entries; i++) {
		union {
			st_data_t data;
			struct aingle_record_field_t *f;
		} fa, fb;
		st_lookup(a->fields, i, &fa.data);
		if (!st_lookup(b->fields, i, &fb.data)) {
			return 0;
		}
		if (strcmp(fa.f->name, fb.f->name)) {
			/*
			 * They have fields with different names 
			 */
			return 0;
		}
		if (!aingle_schema_equal(fa.f->type, fb.f->type)) {
			/*
			 * They have fields with different schemas 
			 */
			return 0;
		}
	}
	return 1;
}

static int
schema_enum_equal(struct aingle_enum_schema_t *a, struct aingle_enum_schema_t *b)
{
	long i;
	if (strcmp(a->name, b->name)) {
		/*
		 * They have different names 
		 */
		return 0;
	}
	if (nullstrcmp(a->space, b->space)) {
		return 0;
	}
	for (i = 0; i < a->symbols->num_entries; i++) {
		union {
			st_data_t data;
			char *sym;
		} sa, sb;
		st_lookup(a->symbols, i, &sa.data);
		if (!st_lookup(b->symbols, i, &sb.data)) {
			return 0;
		}
		if (strcmp(sa.sym, sb.sym) != 0) {
			/*
			 * They have different symbol names 
			 */
			return 0;
		}
	}
	return 1;
}

static int
schema_fixed_equal(struct aingle_fixed_schema_t *a, struct aingle_fixed_schema_t *b)
{
	if (strcmp(a->name, b->name)) {
		/*
		 * They have different names 
		 */
		return 0;
	}
	if (nullstrcmp(a->space, b->space)) {
		return 0;
	}
	return (a->size == b->size);
}

static int
schema_map_equal(struct aingle_map_schema_t *a, struct aingle_map_schema_t *b)
{
	return aingle_schema_equal(a->values, b->values);
}

static int
schema_array_equal(struct aingle_array_schema_t *a, struct aingle_array_schema_t *b)
{
	return aingle_schema_equal(a->items, b->items);
}

static int
schema_union_equal(struct aingle_union_schema_t *a, struct aingle_union_schema_t *b)
{
	long i;
	for (i = 0; i < a->branches->num_entries; i++) {
		union {
			st_data_t data;
			aingle_schema_t schema;
		} ab, bb;
		st_lookup(a->branches, i, &ab.data);
		if (!st_lookup(b->branches, i, &bb.data)) {
			return 0;
		}
		if (!aingle_schema_equal(ab.schema, bb.schema)) {
			/*
			 * They don't have the same schema types 
			 */
			return 0;
		}
	}
	return 1;
}

static int
schema_link_equal(struct aingle_link_schema_t *a, struct aingle_link_schema_t *b)
{
	/*
	 * NOTE: links can only be used for named types. They are used in
	 * recursive schemas so we just check the name of the schema pointed
	 * to instead of a deep check.  Otherwise, we recurse forever... 
	 */
	if (is_aingle_record(a->to)) {
		if (!is_aingle_record(b->to)) {
			return 0;
		}
		if (nullstrcmp(aingle_schema_to_record(a->to)->space,
			       aingle_schema_to_record(b->to)->space)) {
			return 0;
		}
	}
	return (strcmp(aingle_schema_name(a->to), aingle_schema_name(b->to)) == 0);
}

int aingle_schema_equal(aingle_schema_t a, aingle_schema_t b)
{
	if (!a || !b) {
		/*
		 * this is an error. protecting from segfault. 
		 */
		return 0;
	} else if (a == b) {
		/*
		 * an object is equal to itself 
		 */
		return 1;
	} else if (aingle_typeof(a) != aingle_typeof(b)) {
		return 0;
	} else if (is_aingle_record(a)) {
		return schema_record_equal(aingle_schema_to_record(a),
					   aingle_schema_to_record(b));
	} else if (is_aingle_enum(a)) {
		return schema_enum_equal(aingle_schema_to_enum(a),
					 aingle_schema_to_enum(b));
	} else if (is_aingle_fixed(a)) {
		return schema_fixed_equal(aingle_schema_to_fixed(a),
					  aingle_schema_to_fixed(b));
	} else if (is_aingle_map(a)) {
		return schema_map_equal(aingle_schema_to_map(a),
					aingle_schema_to_map(b));
	} else if (is_aingle_array(a)) {
		return schema_array_equal(aingle_schema_to_array(a),
					  aingle_schema_to_array(b));
	} else if (is_aingle_union(a)) {
		return schema_union_equal(aingle_schema_to_union(a),
					  aingle_schema_to_union(b));
	} else if (is_aingle_link(a)) {
		return schema_link_equal(aingle_schema_to_link(a),
					 aingle_schema_to_link(b));
	}
	return 1;
}
