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
#include <string.h>
#include <sys/types.h>
#ifdef _WIN32
 #include "msdirent.h"
#else
 #include <dirent.h>
#endif

int test_cases = 0;
aingle_writer_t aingle_stderr;

static void run_tests(char *dirpath, int should_pass)
{
	char jsontext[4096];
	char jsontext2[4096];
	size_t rval;
	char filepath[1024];
	DIR *dir;
	struct dirent *dent;
	FILE *fp;
	aingle_schema_t schema;
	aingle_writer_t jsontext2_writer;

	dir = opendir(dirpath);
	if (dir == NULL) {
		fprintf(stderr, "Unable to open '%s'\n", dirpath);
		exit(EXIT_FAILURE);
	}
	do {
		dent = readdir(dir);

		/* Suppress failures on CVS directories */
		if ( dent && !strcmp( (const char *) dent->d_name, "CVS" ) )
			continue;

		if (dent && dent->d_name[0] != '.') {
			int test_rval;
			snprintf(filepath, sizeof(filepath), "%s/%s", dirpath,
				 dent->d_name);
			fprintf(stderr, "TEST %s...", filepath);
			fp = fopen(filepath, "r");
			if (!fp) {
				fprintf(stderr, "can't open!\n");
				exit(EXIT_FAILURE);
			}
			rval = fread(jsontext, 1, sizeof(jsontext) - 1, fp);
			fclose(fp);
			jsontext[rval] = '\0';
			test_rval =
			    aingle_schema_from_json(jsontext, 0, &schema, NULL);
			test_cases++;
			if (test_rval == 0) {
				if (should_pass) {
					aingle_schema_t schema_copy =
					    aingle_schema_copy(schema);
					fprintf(stderr, "pass\n");
					aingle_schema_to_json(schema,
							    aingle_stderr);
					fprintf(stderr, "\n");
					if (!aingle_schema_equal
					    (schema, schema_copy)) {
						fprintf(stderr,
							"failed to aingle_schema_equal(schema,aingle_schema_copy())\n");
						exit(EXIT_FAILURE);
					}
					jsontext2_writer = aingle_writer_memory(jsontext2, sizeof(jsontext2));
					if (aingle_schema_to_json(schema, jsontext2_writer)) {
						fprintf(stderr, "failed to write schema (%s)\n",
							aingle_strerror());
						exit(EXIT_FAILURE);
					}
					aingle_write(jsontext2_writer, (void *)"", 1);  /* zero terminate */
					aingle_writer_free(jsontext2_writer);
					aingle_schema_decref(schema);
					if (aingle_schema_from_json(jsontext2, 0, &schema, NULL)) {
						fprintf(stderr, "failed to write then read schema (%s)\n",
							aingle_strerror());
						exit(EXIT_FAILURE);
					}
					if (!aingle_schema_equal
					    (schema, schema_copy)) {
						fprintf(stderr, "failed read-write-read cycle (%s)\n",
							aingle_strerror());
						exit(EXIT_FAILURE);
					}
					aingle_schema_decref(schema_copy);
					aingle_schema_decref(schema);
				} else {
					/*
					 * Unexpected success 
					 */
					fprintf(stderr,
						"fail! (shouldn't succeed but did)\n");
					exit(EXIT_FAILURE);
				}
			} else {
				if (should_pass) {
					fprintf(stderr, "%s\n", aingle_strerror());
					fprintf(stderr,
						"fail! (should have succeeded but didn't)\n");
					exit(EXIT_FAILURE);
				} else {
					fprintf(stderr, "pass\n");
				}
			}
		}
	}
	while (dent != NULL);
	closedir(dir);
}

static int test_array(void)
{
	aingle_schema_t schema = aingle_schema_array(aingle_schema_int());

	if (!aingle_schema_equal
	    (aingle_schema_array_items(schema), aingle_schema_int())) {
		fprintf(stderr, "Unexpected array items schema");
		exit(EXIT_FAILURE);
	}

	aingle_schema_decref(schema);
	return 0;
}

static int test_enum(void)
{
	enum aingle_languages {
		AINGLE_C,
		AINGLE_CPP,
		AINGLE_PYTHON,
		AINGLE_RUBY,
		AINGLE_JAVA
	};
	aingle_schema_t schema = aingle_schema_enum("language");

	aingle_schema_enum_symbol_append(schema, "C");
	aingle_schema_enum_symbol_append(schema, "C++");
	aingle_schema_enum_symbol_append(schema, "Python");
	aingle_schema_enum_symbol_append(schema, "Ruby");
	aingle_schema_enum_symbol_append(schema, "Java");

	const char  *symbol1 = aingle_schema_enum_get(schema, 1);
	if (strcmp(symbol1, "C++") != 0) {
		fprintf(stderr, "Unexpected enum schema symbol\n");
		exit(EXIT_FAILURE);
	}

	if (aingle_schema_enum_get_by_name(schema, "C++") != 1) {
		fprintf(stderr, "Unexpected enum schema index\n");
		exit(EXIT_FAILURE);
	}

	if (aingle_schema_enum_get_by_name(schema, "Haskell") != -1) {
		fprintf(stderr, "Unexpected enum schema index\n");
		exit(EXIT_FAILURE);
	}

	aingle_schema_decref(schema);
	return 0;
}

static int test_fixed(void)
{
	aingle_schema_t schema = aingle_schema_fixed("msg", 8);
	if (aingle_schema_fixed_size(schema) != 8) {
		fprintf(stderr, "Unexpected fixed size\n");
		exit(EXIT_FAILURE);
	}

	aingle_schema_decref(schema);
	return 0;
}

static int test_map(void)
{
	aingle_schema_t schema = aingle_schema_map(aingle_schema_long());

	if (!aingle_schema_equal
	    (aingle_schema_map_values(schema), aingle_schema_long())) {
		fprintf(stderr, "Unexpected map values schema");
		exit(EXIT_FAILURE);
	}

	aingle_schema_decref(schema);
	return 0;
}

static int test_record(void)
{
	aingle_schema_t schema = aingle_schema_record("person", NULL);

	aingle_schema_record_field_append(schema, "name", aingle_schema_string());
	aingle_schema_record_field_append(schema, "age", aingle_schema_int());

	if (aingle_schema_record_field_get_index(schema, "name") != 0) {
		fprintf(stderr, "Incorrect index for \"name\" field\n");
		exit(EXIT_FAILURE);
	}

	if (aingle_schema_record_field_get_index(schema, "unknown") != -1) {
		fprintf(stderr, "Incorrect index for \"unknown\" field\n");
		exit(EXIT_FAILURE);
	}

	aingle_schema_t  name_field =
		aingle_schema_record_field_get(schema, "name");
	if (!aingle_schema_equal(name_field, aingle_schema_string())) {
		fprintf(stderr, "Unexpected name field\n");
		exit(EXIT_FAILURE);
	}

	aingle_schema_t  field1 =
		aingle_schema_record_field_get_by_index(schema, 1);
	if (!aingle_schema_equal(field1, aingle_schema_int())) {
		fprintf(stderr, "Unexpected field 1\n");
		exit(EXIT_FAILURE);
	}

	aingle_schema_decref(schema);
	return 0;
}

static int test_union(void)
{
	aingle_schema_t schema = aingle_schema_union();

	aingle_schema_union_append(schema, aingle_schema_string());
	aingle_schema_union_append(schema, aingle_schema_int());
	aingle_schema_union_append(schema, aingle_schema_null());

	if (!aingle_schema_equal
	    (aingle_schema_string(),
	     aingle_schema_union_branch(schema, 0))) {
		fprintf(stderr, "Unexpected union schema branch 0\n");
		exit(EXIT_FAILURE);
	}

	if (!aingle_schema_equal
	    (aingle_schema_string(),
	     aingle_schema_union_branch_by_name(schema, NULL, "string"))) {
		fprintf(stderr, "Unexpected union schema branch \"string\"\n");
		exit(EXIT_FAILURE);
	}

	aingle_schema_decref(schema);
	return 0;
}

int main(int argc, char *argv[])
{
	char *srcdir = getenv("srcdir");
	char path[1024];

	AINGLE_UNUSED(argc);
	AINGLE_UNUSED(argv);

	if (!srcdir) {
		srcdir = ".";
	}

	aingle_stderr = aingle_writer_file(stderr);

	/*
	 * Run the tests that should pass 
	 */
	snprintf(path, sizeof(path), "%s/schema_tests/pass", srcdir);
	fprintf(stderr, "RUNNING %s\n", path);
	run_tests(path, 1);
	snprintf(path, sizeof(path), "%s/schema_tests/fail", srcdir);
	fprintf(stderr, "RUNNING %s\n", path);
	run_tests(path, 0);

	fprintf(stderr, "*** Running array tests **\n");
	test_array();
	fprintf(stderr, "*** Running enum tests **\n");
	test_enum();
	fprintf(stderr, "*** Running fixed tests **\n");
	test_fixed();
	fprintf(stderr, "*** Running map tests **\n");
	test_map();
	fprintf(stderr, "*** Running record tests **\n");
	test_record();
	fprintf(stderr, "*** Running union tests **\n");
	test_union();

	fprintf(stderr, "==================================================\n");
	fprintf(stderr,
		"Finished running %d schema test cases successfully \n",
		test_cases);
	fprintf(stderr, "==================================================\n");

	aingle_writer_free(aingle_stderr);
	return EXIT_SUCCESS;
}
