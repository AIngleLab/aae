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
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "schema.h"
#include "datum.h"
#include "encoding.h"

#define size_check(rval, call) { rval = call; if(rval) return rval; }
#define size_accum(rval, size, call) { rval = call; if (rval < 0) return rval; else size += rval; }

static int64_t size_datum(aingle_writer_t writer, const aingle_encoding_t * enc,
			  aingle_schema_t writers_schema, aingle_datum_t datum);

static int64_t
size_record(aingle_writer_t writer, const aingle_encoding_t * enc,
	    struct aingle_record_schema_t *schema, aingle_datum_t datum)
{
	int rval;
	long i;
	int64_t size;
	aingle_datum_t field_datum;

	size = 0;
	if (schema) {
		for (i = 0; i < schema->fields->num_entries; i++) {
			union {
				st_data_t data;
				struct aingle_record_field_t *field;
			} val;
			st_lookup(schema->fields, i, &val.data);
			size_check(rval,
				   aingle_record_get(datum, val.field->name,
						   &field_datum));
			size_accum(rval, size,
				   size_datum(writer, enc, val.field->type,
					      field_datum));
		}
	} else {
		/* No schema.  Just write the record datum */
		struct aingle_record_datum_t *record =
		    aingle_datum_to_record(datum);
		for (i = 0; i < record->field_order->num_entries; i++) {
			union {
				st_data_t data;
				char *name;
			} val;
			st_lookup(record->field_order, i, &val.data);
			size_check(rval,
				   aingle_record_get(datum, val.name,
						   &field_datum));
			size_accum(rval, size,
				   size_datum(writer, enc, NULL, field_datum));
		}
	}
	return size;
}

static int64_t
size_enum(aingle_writer_t writer, const aingle_encoding_t * enc,
	  struct aingle_enum_schema_t *enump, struct aingle_enum_datum_t *datum)
{
	AINGLE_UNUSED(enump);

	return enc->size_long(writer, datum->value);
}

struct size_map_args {
	int rval;
	int64_t size;
	aingle_writer_t writer;
	const aingle_encoding_t *enc;
	aingle_schema_t values_schema;
};

static int
size_map_foreach(char *key, aingle_datum_t datum, struct size_map_args *args)
{
	int rval = args->enc->size_string(args->writer, key);
	if (rval < 0) {
		args->rval = rval;
		return ST_STOP;
	} else {
		args->size += rval;
	}
	rval = size_datum(args->writer, args->enc, args->values_schema, datum);
	if (rval < 0) {
		args->rval = rval;
		return ST_STOP;
	} else {
		args->size += rval;
	}
	return ST_CONTINUE;
}

static int64_t
size_map(aingle_writer_t writer, const aingle_encoding_t * enc,
	 struct aingle_map_schema_t *writers_schema,
	 struct aingle_map_datum_t *datum)
{
	int rval;
	int64_t size;
	struct size_map_args args = { 0, 0, writer, enc,
		writers_schema ? writers_schema->values : NULL
	};

	size = 0;
	if (datum->map->num_entries) {
		size_accum(rval, size,
			   enc->size_long(writer, datum->map->num_entries));
		st_foreach(datum->map, HASH_FUNCTION_CAST size_map_foreach, (st_data_t) & args);
		size += args.size;
	}
	if (!args.rval) {
		size_accum(rval, size, enc->size_long(writer, 0));
	}
	return size;
}

static int64_t
size_array(aingle_writer_t writer, const aingle_encoding_t * enc,
	   struct aingle_array_schema_t *schema, struct aingle_array_datum_t *array)
{
	int rval;
	long i;
	int64_t size;

	size = 0;
	if (array->els->num_entries) {
		size_accum(rval, size,
			   enc->size_long(writer, array->els->num_entries));
		for (i = 0; i < array->els->num_entries; i++) {
			union {
				st_data_t data;
				aingle_datum_t datum;
			} val;
			st_lookup(array->els, i, &val.data);
			size_accum(rval, size,
				   size_datum(writer, enc,
					      schema ? schema->items : NULL,
					      val.datum));
		}
	}
	size_accum(rval, size, enc->size_long(writer, 0));
	return size;
}

static int64_t
size_union(aingle_writer_t writer, const aingle_encoding_t * enc,
	   struct aingle_union_schema_t *schema,
	   struct aingle_union_datum_t *unionp)
{
	int rval;
	int64_t size;
	aingle_schema_t write_schema = NULL;

	size = 0;
	size_accum(rval, size, enc->size_long(writer, unionp->discriminant));
	if (schema) {
		write_schema =
		    aingle_schema_union_branch(&schema->obj, unionp->discriminant);
		if (!write_schema) {
			return -EINVAL;
		}
	}
	size_accum(rval, size,
		   size_datum(writer, enc, write_schema, unionp->value));
	return size;
}

static int64_t size_datum(aingle_writer_t writer, const aingle_encoding_t * enc,
			  aingle_schema_t writers_schema, aingle_datum_t datum)
{
	if (is_aingle_schema(writers_schema) && is_aingle_link(writers_schema)) {
		return size_datum(writer, enc,
				  (aingle_schema_to_link(writers_schema))->to,
				  datum);
	}

	switch (aingle_typeof(datum)) {
	case AINGLE_NULL:
		return enc->size_null(writer);

	case AINGLE_BOOLEAN:
		return enc->size_boolean(writer,
					 aingle_datum_to_boolean(datum)->i);

	case AINGLE_STRING:
		return enc->size_string(writer, aingle_datum_to_string(datum)->s);

	case AINGLE_BYTES:
		return enc->size_bytes(writer,
				       aingle_datum_to_bytes(datum)->bytes,
				       aingle_datum_to_bytes(datum)->size);

	case AINGLE_INT32:
	case AINGLE_INT64:{
			int64_t val = aingle_typeof(datum) == AINGLE_INT32 ?
			    aingle_datum_to_int32(datum)->i32 :
			    aingle_datum_to_int64(datum)->i64;
			if (is_aingle_schema(writers_schema)) {
				/* handle promotion */
				if (is_aingle_float(writers_schema)) {
					return enc->size_float(writer,
							       (float)val);
				} else if (is_aingle_double(writers_schema)) {
					return enc->size_double(writer,
								(double)val);
				}
			}
			return enc->size_long(writer, val);
		}

	case AINGLE_FLOAT:{
			float val = aingle_datum_to_float(datum)->f;
			if (is_aingle_schema(writers_schema)
			    && is_aingle_double(writers_schema)) {
				/* handle promotion */
				return enc->size_double(writer, (double)val);
			}
			return enc->size_float(writer, val);
		}

	case AINGLE_DOUBLE:
		return enc->size_double(writer, aingle_datum_to_double(datum)->d);

	case AINGLE_RECORD:
		return size_record(writer, enc,
				   aingle_schema_to_record(writers_schema),
				   datum);

	case AINGLE_ENUM:
		return size_enum(writer, enc,
				 aingle_schema_to_enum(writers_schema),
				 aingle_datum_to_enum(datum));

	case AINGLE_FIXED:
		return aingle_datum_to_fixed(datum)->size;

	case AINGLE_MAP:
		return size_map(writer, enc,
				aingle_schema_to_map(writers_schema),
				aingle_datum_to_map(datum));

	case AINGLE_ARRAY:
		return size_array(writer, enc,
				  aingle_schema_to_array(writers_schema),
				  aingle_datum_to_array(datum));

	case AINGLE_UNION:
		return size_union(writer, enc,
				  aingle_schema_to_union(writers_schema),
				  aingle_datum_to_union(datum));

	case AINGLE_LINK:
		break;
	}

	return 0;
}

int64_t aingle_size_data(aingle_writer_t writer, aingle_schema_t writers_schema,
		       aingle_datum_t datum)
{
	check_param(-EINVAL, writer, "writer");
	check_param(-EINVAL, is_aingle_datum(datum), "datum");
	/* Only validate datum if a writer's schema is provided */
	if (is_aingle_schema(writers_schema)
	    && !aingle_schema_datum_validate(writers_schema, datum)) {
		aingle_set_error("Datum doesn't validate against schema");
		return -EINVAL;
	}
	return size_datum(writer, &aingle_binary_encoding, writers_schema, datum);
}
