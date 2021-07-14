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

#include "aingle.h"
#include "aingle_private.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int rval;
	aingle_file_writer_t file_writer;
	aingle_file_reader_t file_reader;
	char outpath[128];
	FILE *fp;
	char jsontext[16 * 1024];
	aingle_schema_t schema;
	aingle_schema_error_t schema_error;
	aingle_datum_t interop;
	aingle_datum_t array_datum;
	aingle_datum_t node_datum;
	aingle_datum_t union_datum;
	aingle_datum_t out_datum;
	enum Kind {
		KIND_A,
		KIND_B,
		KIND_C
	};

	if (argc != 3) {
		exit(EXIT_FAILURE);
	}
	snprintf(outpath, sizeof(outpath), "%s/c.aingle", argv[2]);
	fprintf(stderr, "Writing to %s\n", outpath);

	fp = fopen(argv[1], "r");
	rval = fread(jsontext, 1, sizeof(jsontext) - 1, fp);
	jsontext[rval] = '\0';

	check(rval,
	      aingle_schema_from_json(jsontext, rval, &schema, &schema_error));
	check(rval, aingle_file_writer_create(outpath, schema, &file_writer));

	/* TODO: create a method for generating random data from schema */
	interop = aingle_record(schema);
	aingle_record_set(interop, "intField", aingle_int32(42));
	aingle_record_set(interop, "longField", aingle_int64(4242));
	aingle_record_set(interop, "stringField",
			aingle_givestring("Follow your bliss.", NULL));
	aingle_record_set(interop, "boolField", aingle_boolean(1));
	aingle_record_set(interop, "floatField", aingle_float(3.14159265));
	aingle_record_set(interop, "doubleField", aingle_double(2.71828183));
	aingle_record_set(interop, "bytesField", aingle_bytes("abcd", 4));
	aingle_record_set(interop, "nullField", aingle_null());

	aingle_schema_t  array_schema = aingle_schema_get_subschema(schema, "arrayField");
	array_datum = aingle_array(array_schema);
	aingle_array_append_datum(array_datum, aingle_double(1.0));
	aingle_array_append_datum(array_datum, aingle_double(2.0));
	aingle_array_append_datum(array_datum, aingle_double(3.0));
	aingle_record_set(interop, "arrayField", array_datum);

	aingle_schema_t  map_schema = aingle_schema_get_subschema(schema, "mapField");
	aingle_record_set(interop, "mapField", aingle_map(map_schema));

	aingle_schema_t  union_schema = aingle_schema_get_subschema(schema, "unionField");
	union_datum = aingle_union(union_schema, 1, aingle_double(1.61803399));
	aingle_record_set(interop, "unionField", union_datum);

	aingle_schema_t  enum_schema = aingle_schema_get_subschema(schema, "enumField");
	aingle_record_set(interop, "enumField", aingle_enum(enum_schema, KIND_A));

	aingle_schema_t  fixed_schema = aingle_schema_get_subschema(schema, "fixedField");
	aingle_record_set(interop, "fixedField",
			aingle_fixed(fixed_schema, "1234567890123456", 16));

	aingle_schema_t  node_schema = aingle_schema_get_subschema(schema, "recordField");
	node_datum = aingle_record(node_schema);
	aingle_record_set(node_datum, "label",
			aingle_givestring("If you label me, you negate me.", NULL));
	aingle_schema_t  children_schema = aingle_schema_get_subschema(node_schema, "children");
	aingle_record_set(node_datum, "children", aingle_array(children_schema));
	aingle_record_set(interop, "recordField", node_datum);

	rval = aingle_file_writer_append(file_writer, interop);
	if (rval) {
		fprintf(stderr, "Unable to append data to interop file!\n");
		exit(EXIT_FAILURE);
	} else {
		fprintf(stderr, "Successfully appended datum to file\n");
	}

	check(rval, aingle_file_writer_close(file_writer));
	fprintf(stderr, "Closed writer.\n");

	check(rval, aingle_file_reader(outpath, &file_reader));
	fprintf(stderr, "Re-reading datum to verify\n");
	check(rval, aingle_file_reader_read(file_reader, NULL, &out_datum));
	fprintf(stderr, "Verifying datum...");
	if (!aingle_datum_equal(interop, out_datum)) {
		fprintf(stderr, "fail!\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "ok\n");
	check(rval, aingle_file_reader_close(file_reader));
	fprintf(stderr, "Closed reader.\n");
	return 0;
}
