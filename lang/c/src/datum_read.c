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

#include <stdlib.h>
#include <errno.h>

#include "aingle/errors.h"
#include "aingle/io.h"
#include "aingle/legacy.h"
#include "aingle/resolver.h"
#include "aingle/schema.h"
#include "aingle/value.h"
#include "aingle_private.h"

int
aingle_schema_match(aingle_schema_t wschema, aingle_schema_t rschema)
{
	check_param(0, is_aingle_schema(wschema), "writer schema");
	check_param(0, is_aingle_schema(rschema), "reader schema");

	aingle_value_iface_t  *resolver =
	    aingle_resolved_writer_new(wschema, rschema);
	if (resolver != NULL) {
		aingle_value_iface_decref(resolver);
		return 1;
	}

	return 0;
}

int
aingle_read_data(aingle_reader_t reader, aingle_schema_t writers_schema,
	       aingle_schema_t readers_schema, aingle_datum_t * datum)
{
	int rval;

	check_param(EINVAL, reader, "reader");
	check_param(EINVAL, is_aingle_schema(writers_schema), "writer schema");
	check_param(EINVAL, datum, "datum pointer");

	if (!readers_schema) {
		readers_schema = writers_schema;
	}

	aingle_datum_t  result = aingle_datum_from_schema(readers_schema);
	if (!result) {
		return EINVAL;
	}

	aingle_value_t  value;
	check(rval, aingle_datum_as_value(&value, result));

	aingle_value_iface_t  *resolver =
	    aingle_resolved_writer_new(writers_schema, readers_schema);
	if (!resolver) {
		aingle_value_decref(&value);
		aingle_datum_decref(result);
		return EINVAL;
	}

	aingle_value_t  resolved_value;
	rval = aingle_resolved_writer_new_value(resolver, &resolved_value);
	if (rval) {
		aingle_value_iface_decref(resolver);
		aingle_value_decref(&value);
		aingle_datum_decref(result);
		return rval;
	}

	aingle_resolved_writer_set_dest(&resolved_value, &value);
	rval = aingle_value_read(reader, &resolved_value);
	if (rval) {
		aingle_value_decref(&resolved_value);
		aingle_value_iface_decref(resolver);
		aingle_value_decref(&value);
		aingle_datum_decref(result);
		return rval;
	}

	aingle_value_decref(&resolved_value);
	aingle_value_iface_decref(resolver);
	aingle_value_decref(&value);
	*datum = result;
	return 0;
}
