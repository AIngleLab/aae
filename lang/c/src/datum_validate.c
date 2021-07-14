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
#include "aingle/errors.h"
#include <limits.h>
#include <errno.h>
#include <string.h>
#include "schema.h"
#include "datum.h"
#include "st.h"

struct validate_st {
	aingle_schema_t expected_schema;
	int rval;
};

static int
schema_map_validate_foreach(char *key, aingle_datum_t datum,
			    struct validate_st *vst)
{
	AINGLE_UNUSED(key);

	if (!aingle_schema_datum_validate(vst->expected_schema, datum)) {
		vst->rval = 0;
		return ST_STOP;
	}
	return ST_CONTINUE;
}

int
aingle_schema_datum_validate(aingle_schema_t expected_schema, aingle_datum_t datum)
{
	check_param(EINVAL, expected_schema, "expected schema");
	check_param(EINVAL, is_aingle_datum(datum), "datum");

	int rval;
	long i;

	switch (aingle_typeof(expected_schema)) {
	case AINGLE_NULL:
		return is_aingle_null(datum);

	case AINGLE_BOOLEAN:
		return is_aingle_boolean(datum);

	case AINGLE_STRING:
		return is_aingle_string(datum);

	case AINGLE_BYTES:
		return is_aingle_bytes(datum);

	case AINGLE_INT32:
		return is_aingle_int32(datum)
		    || (is_aingle_int64(datum)
			&& (INT_MIN <= aingle_datum_to_int64(datum)->i64
			    && aingle_datum_to_int64(datum)->i64 <= INT_MAX));

	case AINGLE_INT64:
		return is_aingle_int32(datum) || is_aingle_int64(datum);

	case AINGLE_FLOAT:
		return is_aingle_int32(datum) || is_aingle_int64(datum)
		    || is_aingle_float(datum);

	case AINGLE_DOUBLE:
		return is_aingle_int32(datum) || is_aingle_int64(datum)
		    || is_aingle_float(datum) || is_aingle_double(datum);

	case AINGLE_FIXED:
		return (is_aingle_fixed(datum)
			&& (aingle_schema_to_fixed(expected_schema)->size ==
			    aingle_datum_to_fixed(datum)->size));

	case AINGLE_ENUM:
		if (is_aingle_enum(datum)) {
			long value = aingle_datum_to_enum(datum)->value;
			long max_value =
			    aingle_schema_to_enum(expected_schema)->symbols->
			    num_entries;
			return 0 <= value && value <= max_value;
		}
		return 0;

	case AINGLE_ARRAY:
		if (is_aingle_array(datum)) {
			struct aingle_array_datum_t *array =
			    aingle_datum_to_array(datum);

			for (i = 0; i < array->els->num_entries; i++) {
				union {
					st_data_t data;
					aingle_datum_t datum;
				} val;
				st_lookup(array->els, i, &val.data);
				if (!aingle_schema_datum_validate
				    ((aingle_schema_to_array
				      (expected_schema))->items, val.datum)) {
					return 0;
				}
			}
			return 1;
		}
		return 0;

	case AINGLE_MAP:
		if (is_aingle_map(datum)) {
			struct validate_st vst =
			    { aingle_schema_to_map(expected_schema)->values, 1
			};
			st_foreach(aingle_datum_to_map(datum)->map,
				   HASH_FUNCTION_CAST schema_map_validate_foreach,
				   (st_data_t) & vst);
			return vst.rval;
		}
		break;

	case AINGLE_UNION:
		if (is_aingle_union(datum)) {
			struct aingle_union_schema_t *union_schema =
			    aingle_schema_to_union(expected_schema);
			struct aingle_union_datum_t *union_datum =
			    aingle_datum_to_union(datum);
			union {
				st_data_t data;
				aingle_schema_t schema;
			} val;

			if (!st_lookup
			    (union_schema->branches, union_datum->discriminant,
			     &val.data)) {
				return 0;
			}
			return aingle_schema_datum_validate(val.schema,
							  union_datum->value);
		}
		break;

	case AINGLE_RECORD:
		if (is_aingle_record(datum)) {
			struct aingle_record_schema_t *record_schema =
			    aingle_schema_to_record(expected_schema);
			for (i = 0; i < record_schema->fields->num_entries; i++) {
				aingle_datum_t field_datum;
				union {
					st_data_t data;
					struct aingle_record_field_t *field;
				} val;
				st_lookup(record_schema->fields, i, &val.data);

				rval =
				    aingle_record_get(datum, val.field->name,
						    &field_datum);
				if (rval) {
					/*
					 * TODO: check for default values 
					 */
					return rval;
				}
				if (!aingle_schema_datum_validate
				    (val.field->type, field_datum)) {
					return 0;
				}
			}
			return 1;
		}
		break;

	case AINGLE_LINK:
		{
			return
			    aingle_schema_datum_validate((aingle_schema_to_link
							(expected_schema))->to,
						       datum);
		}
		break;
	}
	return 0;
}
