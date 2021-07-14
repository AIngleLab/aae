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

#include <aingle.h>
#include <stdio.h>
#include <stdlib.h>

#define check_exit(call) \
	do { \
		int  __rc = call; \
		if (__rc != 0) { \
			fprintf(stderr, "Unexpected error:\n  %s\n  %s\n", \
				aingle_strerror(), #call); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)

#define expect_error(call) \
	do { \
		int  __rc = call; \
		if (__rc == 0) { \
			fprintf(stderr, "Expected an error:\n  %s\n", #call); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)

#define check_expected_value(actual, expected) \
	do { \
		if (!aingle_value_equal_fast((actual), (expected))) { \
			char  *actual_json; \
			char  *expected_json; \
			aingle_value_to_json((actual), 1, &actual_json); \
			aingle_value_to_json((expected), 1, &expected_json); \
			fprintf(stderr, "Expected %s\nGot      %s\n", \
				expected_json, actual_json); \
			free(actual_json); \
			free(expected_json); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)

int main(void)
{
	aingle_schema_t  schema;
	aingle_file_reader_t  reader;
	aingle_value_iface_t  *iface;
	aingle_value_t  actual;
	aingle_value_t  expected;
	aingle_value_t  branch;

	schema = aingle_schema_union();
	aingle_schema_union_append(schema, aingle_schema_null());
	aingle_schema_union_append(schema, aingle_schema_int());

	iface = aingle_generic_class_from_schema(schema);
	aingle_generic_value_new(iface, &actual);
	aingle_generic_value_new(iface, &expected);


	/* First read the contents of the good file. */

	check_exit(aingle_file_reader("aingle-1237-good.aingle", &reader));

	check_exit(aingle_file_reader_read_value(reader, &actual));
	check_exit(aingle_value_set_branch(&expected, 0, &branch));
	check_exit(aingle_value_set_null(&branch));
	check_expected_value(&actual, &expected);

	check_exit(aingle_file_reader_read_value(reader, &actual));
	check_exit(aingle_value_set_branch(&expected, 1, &branch));
	check_exit(aingle_value_set_int(&branch, 100));
	check_expected_value(&actual, &expected);

	check_exit(aingle_file_reader_close(reader));


	/* Then read from the malformed file. */

	check_exit(aingle_file_reader
		   ("aingle-1237-bad-union-discriminant.aingle", &reader));

	check_exit(aingle_file_reader_read_value(reader, &actual));
	check_exit(aingle_value_set_branch(&expected, 0, &branch));
	check_exit(aingle_value_set_null(&branch));
	check_expected_value(&actual, &expected);

	expect_error(aingle_file_reader_read_value(reader, &actual));

	check_exit(aingle_file_reader_close(reader));


	/* Clean up and exit */
	aingle_value_decref(&actual);
	aingle_value_decref(&expected);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(schema);
	exit(EXIT_SUCCESS);
}
