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

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "aingle/basics.h"
#include "aingle/errors.h"
#include "aingle/io.h"
#include "aingle/legacy.h"
#include "aingle/resolver.h"
#include "aingle/schema.h"
#include "aingle/value.h"
#include "aingle_private.h"

int aingle_write_data(aingle_writer_t writer, aingle_schema_t writers_schema,
		    aingle_datum_t datum)
{
	int  rval;

	check_param(EINVAL, writer, "writer");
	check_param(EINVAL, is_aingle_datum(datum), "datum");

	/* Only validate datum if a writer's schema is provided */
	if (is_aingle_schema(writers_schema)) {
	    if (!aingle_schema_datum_validate(writers_schema, datum)) {
		aingle_set_error("Datum doesn't validate against schema");
		return EINVAL;
	    }

	    /*
	     * Some confusing terminology here.  The "writers_schema"
	     * parameter is the schema we want to use to write the data
	     * into the "writer" buffer.  Before doing that, we need to
	     * resolve the datum from its actual schema into this
	     * "writer" schema.  For the purposes of that resolution,
	     * the writer schema is the datum's actual schema, and the
	     * reader schema is our eventual (when writing to the
	     * buffer) "writer" schema.
	     */

	    aingle_schema_t  datum_schema = aingle_datum_get_schema(datum);
	    aingle_value_iface_t  *resolver =
		aingle_resolved_reader_new(datum_schema, writers_schema);
	    if (resolver == NULL) {
		    return EINVAL;
	    }

	    aingle_value_t  value;
	    check(rval, aingle_datum_as_value(&value, datum));

	    aingle_value_t  resolved;
	    rval = aingle_resolved_reader_new_value(resolver, &resolved);
	    if (rval != 0) {
		    aingle_value_decref(&value);
		    aingle_value_iface_decref(resolver);
		    return rval;
	    }

	    aingle_resolved_reader_set_source(&resolved, &value);
	    rval = aingle_value_write(writer, &resolved);
	    aingle_value_decref(&resolved);
	    aingle_value_decref(&value);
	    aingle_value_iface_decref(resolver);
	    return rval;
	}

	/* If we're writing using the datum's actual schema, we don't
	 * need a resolver. */

	aingle_value_t  value;
	check(rval, aingle_datum_as_value(&value, datum));
	check(rval, aingle_value_write(writer, &value));
	aingle_value_decref(&value);
	return 0;
}
