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
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char buf[4096];
aingle_reader_t reader;
aingle_writer_t writer;

typedef int (*aingle_test) (void);

/*
 * Use a custom allocator that verifies that the size that we use to
 * free an object matches the size that we use to allocate it.
 */

static void *
test_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
	AINGLE_UNUSED(ud);
	AINGLE_UNUSED(osize);

	if (nsize == 0) {
		size_t  *size = ((size_t *) ptr) - 1;
		if (osize != *size) {
			fprintf(stderr,
				"Error freeing %p:\n"
				"Size passed to aingle_free (%" PRIsz ") "
				"doesn't match size passed to "
				"aingle_malloc (%" PRIsz ")\n",
				ptr, osize, *size);
			abort();
			//exit(EXIT_FAILURE);
		}
		free(size);
		return NULL;
	} else {
		size_t  real_size = nsize + sizeof(size_t);
		size_t  *old_size = ptr? ((size_t *) ptr)-1: NULL;
		size_t  *size = (size_t *) realloc(old_size, real_size);
		*size = nsize;
		return (size + 1);
	}
}

void init_rand(void)
{
	srand(time(NULL));
}

double rand_number(double from, double to)
{
	double range = to - from;
	return from + ((double)rand() / (RAND_MAX + 1.0)) * range;
}

int64_t rand_int64(void)
{
	return (int64_t) rand_number(LONG_MIN, LONG_MAX);
}

int32_t rand_int32(void)
{
	return (int32_t) rand_number(INT_MIN, INT_MAX);
}

void
write_read_check(aingle_schema_t writers_schema, aingle_datum_t datum,
		 aingle_schema_t readers_schema, aingle_datum_t expected, char *type)
{
	aingle_datum_t datum_out;
	int validate;

	for (validate = 0; validate <= 1; validate++) {

		reader = aingle_reader_memory(buf, sizeof(buf));
		writer = aingle_writer_memory(buf, sizeof(buf));

		if (!expected) {
			expected = datum;
		}

		/* Validating read/write */
		if (aingle_write_data
		    (writer, validate ? writers_schema : NULL, datum)) {
			fprintf(stderr, "Unable to write %s validate=%d\n  %s\n",
				type, validate, aingle_strerror());
			exit(EXIT_FAILURE);
		}
		int64_t size =
		    aingle_size_data(writer, validate ? writers_schema : NULL,
				   datum);
		if (size != aingle_writer_tell(writer)) {
			fprintf(stderr,
				"Unable to calculate size %s validate=%d "
				"(%"PRId64" != %"PRId64")\n  %s\n",
				type, validate, size, aingle_writer_tell(writer),
				aingle_strerror());
			exit(EXIT_FAILURE);
		}
		if (aingle_read_data
		    (reader, writers_schema, readers_schema, &datum_out)) {
			fprintf(stderr, "Unable to read %s validate=%d\n  %s\n",
				type, validate, aingle_strerror());
			fprintf(stderr, "  %s\n", aingle_strerror());
			exit(EXIT_FAILURE);
		}
		if (!aingle_datum_equal(expected, datum_out)) {
			fprintf(stderr,
				"Unable to encode/decode %s validate=%d\n  %s\n",
				type, validate, aingle_strerror());
			exit(EXIT_FAILURE);
		}

		aingle_reader_dump(reader, stderr);
		aingle_datum_decref(datum_out);
		aingle_reader_free(reader);
		aingle_writer_free(writer);
	}
}

static void test_json(aingle_datum_t datum, const char *expected)
{
	char  *json = NULL;
	aingle_datum_to_json(datum, 1, &json);
	if (strcasecmp(json, expected) != 0) {
		fprintf(stderr, "Unexpected JSON encoding: %s\n", json);
		exit(EXIT_FAILURE);
	}
	free(json);
}

static int test_string(void)
{
	unsigned int i;
	const char *strings[] = { "Four score and seven years ago",
		"our father brought forth on this continent",
		"a new nation", "conceived in Liberty",
		"and dedicated to the proposition that all men are created equal."
	};
	aingle_schema_t writer_schema = aingle_schema_string();
	for (i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
		aingle_datum_t datum = aingle_givestring(strings[i], NULL);
		write_read_check(writer_schema, datum, NULL, NULL, "string");
		aingle_datum_decref(datum);
	}

	aingle_datum_t  datum = aingle_givestring(strings[0], NULL);
	test_json(datum, "\"Four score and seven years ago\"");
	aingle_datum_decref(datum);

	// The following should bork if we don't copy the string value
	// correctly (since we'll try to free a static string).

	datum = aingle_string("this should be copied");
	aingle_string_set(datum, "also this");
	aingle_datum_decref(datum);

	aingle_schema_decref(writer_schema);
	return 0;
}

static int test_bytes(void)
{
	char bytes[] = { 0xDE, 0xAD, 0xBE, 0xEF };
	aingle_schema_t writer_schema = aingle_schema_bytes();
	aingle_datum_t datum;
	aingle_datum_t expected_datum;

	datum = aingle_givebytes(bytes, sizeof(bytes), NULL);
	write_read_check(writer_schema, datum, NULL, NULL, "bytes");
	test_json(datum, "\"\\u00de\\u00ad\\u00be\\u00ef\"");
	aingle_datum_decref(datum);
	aingle_schema_decref(writer_schema);

	datum = aingle_givebytes(NULL, 0, NULL);
	aingle_givebytes_set(datum, bytes, sizeof(bytes), NULL);
	expected_datum = aingle_givebytes(bytes, sizeof(bytes), NULL);
	if (!aingle_datum_equal(datum, expected_datum)) {
		fprintf(stderr,
		        "Expected equal bytes instances.\n");
		exit(EXIT_FAILURE);
	}
	aingle_datum_decref(datum);
	aingle_datum_decref(expected_datum);

	// The following should bork if we don't copy the bytes value
	// correctly (since we'll try to free a static string).

	datum = aingle_bytes("original", 8);
	aingle_bytes_set(datum, "alsothis", 8);
	aingle_datum_decref(datum);

	aingle_schema_decref(writer_schema);
	return 0;
}

static int test_int32(void)
{
	int i;
	aingle_schema_t writer_schema = aingle_schema_int();
	aingle_schema_t long_schema = aingle_schema_long();
	aingle_schema_t float_schema = aingle_schema_float();
	aingle_schema_t double_schema = aingle_schema_double();
	for (i = 0; i < 100; i++) {
		int32_t  value = rand_int32();
		aingle_datum_t datum = aingle_int32(value);
		aingle_datum_t long_datum = aingle_int64(value);
		aingle_datum_t float_datum = aingle_float(value);
		aingle_datum_t double_datum = aingle_double(value);
		write_read_check(writer_schema, datum, NULL, NULL, "int");
		write_read_check(writer_schema, datum,
				 long_schema, long_datum, "int->long");
		write_read_check(writer_schema, datum,
				 float_schema, float_datum, "int->float");
		write_read_check(writer_schema, datum,
				 double_schema, double_datum, "int->double");
		aingle_datum_decref(datum);
		aingle_datum_decref(long_datum);
		aingle_datum_decref(float_datum);
		aingle_datum_decref(double_datum);
	}

	aingle_datum_t  datum = aingle_int32(10000);
	test_json(datum, "10000");
	aingle_datum_decref(datum);

	aingle_schema_decref(writer_schema);
	aingle_schema_decref(long_schema);
	aingle_schema_decref(float_schema);
	aingle_schema_decref(double_schema);
	return 0;
}

static int test_int64(void)
{
	int i;
	aingle_schema_t writer_schema = aingle_schema_long();
	aingle_schema_t float_schema = aingle_schema_float();
	aingle_schema_t double_schema = aingle_schema_double();
	for (i = 0; i < 100; i++) {
		int64_t  value = rand_int64();
		aingle_datum_t datum = aingle_int64(value);
		aingle_datum_t float_datum = aingle_float(value);
		aingle_datum_t double_datum = aingle_double(value);
		write_read_check(writer_schema, datum, NULL, NULL, "long");
		write_read_check(writer_schema, datum,
				 float_schema, float_datum, "long->float");
		write_read_check(writer_schema, datum,
				 double_schema, double_datum, "long->double");
		aingle_datum_decref(datum);
		aingle_datum_decref(float_datum);
		aingle_datum_decref(double_datum);
	}

	aingle_datum_t  datum = aingle_int64(10000);
	test_json(datum, "10000");
	aingle_datum_decref(datum);

	aingle_schema_decref(writer_schema);
	aingle_schema_decref(float_schema);
	aingle_schema_decref(double_schema);
	return 0;
}

static int test_double(void)
{
	int i;
	aingle_schema_t schema = aingle_schema_double();
	for (i = 0; i < 100; i++) {
		aingle_datum_t datum = aingle_double(rand_number(-1.0E10, 1.0E10));
		write_read_check(schema, datum, NULL, NULL, "double");
		aingle_datum_decref(datum);
	}

	aingle_datum_t  datum = aingle_double(2000.0);
	test_json(datum, "2000.0");
	aingle_datum_decref(datum);

	aingle_schema_decref(schema);
	return 0;
}

static int test_float(void)
{
	int i;
	aingle_schema_t schema = aingle_schema_float();
	aingle_schema_t double_schema = aingle_schema_double();
	for (i = 0; i < 100; i++) {
		float  value = rand_number(-1.0E10, 1.0E10);
		aingle_datum_t datum = aingle_float(value);
		aingle_datum_t double_datum = aingle_double(value);
		write_read_check(schema, datum, NULL, NULL, "float");
		write_read_check(schema, datum,
				 double_schema, double_datum, "float->double");
		aingle_datum_decref(datum);
		aingle_datum_decref(double_datum);
	}

	aingle_datum_t  datum = aingle_float(2000.0);
	test_json(datum, "2000.0");
	aingle_datum_decref(datum);

	aingle_schema_decref(schema);
	aingle_schema_decref(double_schema);
	return 0;
}

static int test_boolean(void)
{
	int i;
	const char  *expected_json[] = { "false", "true" };
	aingle_schema_t schema = aingle_schema_boolean();
	for (i = 0; i <= 1; i++) {
		aingle_datum_t datum = aingle_boolean(i);
		write_read_check(schema, datum, NULL, NULL, "boolean");
		test_json(datum, expected_json[i]);
		aingle_datum_decref(datum);
	}
	aingle_schema_decref(schema);
	return 0;
}

static int test_null(void)
{
	aingle_schema_t schema = aingle_schema_null();
	aingle_datum_t datum = aingle_null();
	write_read_check(schema, datum, NULL, NULL, "null");
	test_json(datum, "null");
	aingle_datum_decref(datum);
	return 0;
}

static int test_record(void)
{
	aingle_schema_t schema = aingle_schema_record("person", NULL);
	aingle_schema_record_field_append(schema, "name", aingle_schema_string());
	aingle_schema_record_field_append(schema, "age", aingle_schema_int());

	aingle_datum_t datum = aingle_record(schema);
	aingle_datum_t name_datum, age_datum;

	name_datum = aingle_givestring("Joseph Campbell", NULL);
	age_datum = aingle_int32(83);

	aingle_record_set(datum, "name", name_datum);
	aingle_record_set(datum, "age", age_datum);

	write_read_check(schema, datum, NULL, NULL, "record");
	test_json(datum, "{\"name\": \"Joseph Campbell\", \"age\": 83}");

	int  rc;
	aingle_record_set_field_value(rc, datum, int32, "age", 104);

	int32_t  age = 0;
	aingle_record_get_field_value(rc, datum, int32, "age", &age);
	if (age != 104) {
		fprintf(stderr, "Incorrect age value\n");
		exit(EXIT_FAILURE);
	}

	aingle_datum_decref(name_datum);
	aingle_datum_decref(age_datum);
	aingle_datum_decref(datum);
	aingle_schema_decref(schema);
	return 0;
}

static int test_nested_record(void)
{
	const char  *json =
		"{"
		"  \"type\": \"record\","
		"  \"name\": \"list\","
		"  \"fields\": ["
		"    { \"name\": \"x\", \"type\": \"int\" },"
		"    { \"name\": \"y\", \"type\": \"int\" },"
		"    { \"name\": \"next\", \"type\": [\"null\",\"list\"]}"
		"  ]"
		"}";

	int  rval;

	aingle_schema_t schema = NULL;
	aingle_schema_error_t error;
	aingle_schema_from_json(json, strlen(json), &schema, &error);

	aingle_datum_t  head = aingle_datum_from_schema(schema);
	aingle_record_set_field_value(rval, head, int32, "x", 10);
	aingle_record_set_field_value(rval, head, int32, "y", 10);

	aingle_datum_t  next = NULL;
	aingle_datum_t  tail = NULL;

	aingle_record_get(head, "next", &next);
	aingle_union_set_discriminant(next, 1, &tail);
	aingle_record_set_field_value(rval, tail, int32, "x", 20);
	aingle_record_set_field_value(rval, tail, int32, "y", 20);

	aingle_record_get(tail, "next", &next);
	aingle_union_set_discriminant(next, 0, NULL);

	write_read_check(schema, head, NULL, NULL, "nested record");

	aingle_schema_decref(schema);
	aingle_datum_decref(head);

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
	aingle_datum_t datum = aingle_enum(schema, AINGLE_C);

	aingle_schema_enum_symbol_append(schema, "C");
	aingle_schema_enum_symbol_append(schema, "C++");
	aingle_schema_enum_symbol_append(schema, "Python");
	aingle_schema_enum_symbol_append(schema, "Ruby");
	aingle_schema_enum_symbol_append(schema, "Java");

	if (aingle_enum_get(datum) != AINGLE_C) {
		fprintf(stderr, "Unexpected enum value AINGLE_C\n");
		exit(EXIT_FAILURE);
	}

	if (strcmp(aingle_enum_get_name(datum), "C") != 0) {
		fprintf(stderr, "Unexpected enum value name C\n");
		exit(EXIT_FAILURE);
	}

	write_read_check(schema, datum, NULL, NULL, "enum");
	test_json(datum, "\"C\"");

	aingle_enum_set(datum, AINGLE_CPP);
	if (strcmp(aingle_enum_get_name(datum), "C++") != 0) {
		fprintf(stderr, "Unexpected enum value name C++\n");
		exit(EXIT_FAILURE);
	}

	write_read_check(schema, datum, NULL, NULL, "enum");
	test_json(datum, "\"C++\"");

	aingle_enum_set_name(datum, "Python");
	if (aingle_enum_get(datum) != AINGLE_PYTHON) {
		fprintf(stderr, "Unexpected enum value AINGLE_PYTHON\n");
		exit(EXIT_FAILURE);
	}

	write_read_check(schema, datum, NULL, NULL, "enum");
	test_json(datum, "\"Python\"");

	aingle_datum_decref(datum);
	aingle_schema_decref(schema);
	return 0;
}

static int test_array(void)
{
	int i, rval;
	aingle_schema_t schema = aingle_schema_array(aingle_schema_int());
	aingle_datum_t datum = aingle_array(schema);

	for (i = 0; i < 10; i++) {
		aingle_datum_t i32_datum = aingle_int32(i);
		rval = aingle_array_append_datum(datum, i32_datum);
		aingle_datum_decref(i32_datum);
		if (rval) {
			exit(EXIT_FAILURE);
		}
	}

	if (aingle_array_size(datum) != 10) {
		fprintf(stderr, "Unexpected array size");
		exit(EXIT_FAILURE);
	}

	write_read_check(schema, datum, NULL, NULL, "array");
	test_json(datum, "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]");
	aingle_datum_decref(datum);
	aingle_schema_decref(schema);
	return 0;
}

static int test_map(void)
{
	aingle_schema_t schema = aingle_schema_map(aingle_schema_long());
	aingle_datum_t datum = aingle_map(schema);
	int64_t i = 0;
	char *nums[] =
	    { "zero", "one", "two", "three", "four", "five", "six", NULL };
	while (nums[i]) {
		aingle_datum_t i_datum = aingle_int64(i);
		aingle_map_set(datum, nums[i], i_datum);
		aingle_datum_decref(i_datum);
		i++;
	}

	if (aingle_array_size(datum) != 7) {
		fprintf(stderr, "Unexpected map size\n");
		exit(EXIT_FAILURE);
	}

	aingle_datum_t value;
	const char  *key;
	aingle_map_get_key(datum, 2, &key);
	aingle_map_get(datum, key, &value);
	int64_t  val;
	aingle_int64_get(value, &val);

	if (val != 2) {
		fprintf(stderr, "Unexpected map value 2\n");
		exit(EXIT_FAILURE);
	}

	int  index;
	if (aingle_map_get_index(datum, "two", &index)) {
		fprintf(stderr, "Can't get index for key \"two\": %s\n",
			aingle_strerror());
		exit(EXIT_FAILURE);
	}
	if (index != 2) {
		fprintf(stderr, "Unexpected index for key \"two\"\n");
		exit(EXIT_FAILURE);
	}
	if (!aingle_map_get_index(datum, "foobar", &index)) {
		fprintf(stderr, "Unexpected index for key \"foobar\"\n");
		exit(EXIT_FAILURE);
	}

	write_read_check(schema, datum, NULL, NULL, "map");
	test_json(datum,
		  "{\"zero\": 0, \"one\": 1, \"two\": 2, \"three\": 3, "
		  "\"four\": 4, \"five\": 5, \"six\": 6}");
	aingle_datum_decref(datum);
	aingle_schema_decref(schema);
	return 0;
}

static int test_union(void)
{
	aingle_schema_t schema = aingle_schema_union();
	aingle_datum_t union_datum;
	aingle_datum_t datum;
	aingle_datum_t union_datum1;
	aingle_datum_t datum1;

	aingle_schema_union_append(schema, aingle_schema_string());
	aingle_schema_union_append(schema, aingle_schema_int());
	aingle_schema_union_append(schema, aingle_schema_null());

	datum = aingle_givestring("Follow your bliss.", NULL);
	union_datum = aingle_union(schema, 0, datum);

	if (aingle_union_discriminant(union_datum) != 0) {
		fprintf(stderr, "Unexpected union discriminant\n");
		exit(EXIT_FAILURE);
	}

	if (aingle_union_current_branch(union_datum) != datum) {
		fprintf(stderr, "Unexpected union branch datum\n");
		exit(EXIT_FAILURE);
	}

	union_datum1 = aingle_datum_from_schema(schema);
	aingle_union_set_discriminant(union_datum1, 0, &datum1);
	aingle_givestring_set(datum1, "Follow your bliss.", NULL);

	if (!aingle_datum_equal(datum, datum1)) {
		fprintf(stderr, "Union values should be equal\n");
		exit(EXIT_FAILURE);
	}

	write_read_check(schema, union_datum, NULL, NULL, "union");
	test_json(union_datum, "{\"string\": \"Follow your bliss.\"}");

	aingle_datum_decref(datum);
	aingle_union_set_discriminant(union_datum, 2, &datum);
	test_json(union_datum, "null");

	aingle_datum_decref(union_datum);
	aingle_datum_decref(datum);
	aingle_datum_decref(union_datum1);
	aingle_schema_decref(schema);
	return 0;
}

static int test_fixed(void)
{
	char bytes[] = { 0xD, 0xA, 0xD, 0xA, 0xB, 0xA, 0xB, 0xA };
	aingle_schema_t schema = aingle_schema_fixed("msg", sizeof(bytes));
	aingle_datum_t datum;
	aingle_datum_t expected_datum;

	datum = aingle_givefixed(schema, bytes, sizeof(bytes), NULL);
	write_read_check(schema, datum, NULL, NULL, "fixed");
	test_json(datum, "\"\\r\\n\\r\\n\\u000b\\n\\u000b\\n\"");
	aingle_datum_decref(datum);

	datum = aingle_givefixed(schema, NULL, sizeof(bytes), NULL);
	aingle_givefixed_set(datum, bytes, sizeof(bytes), NULL);
	expected_datum = aingle_givefixed(schema, bytes, sizeof(bytes), NULL);
	if (!aingle_datum_equal(datum, expected_datum)) {
		fprintf(stderr,
		        "Expected equal fixed instances.\n");
		exit(EXIT_FAILURE);
	}
	aingle_datum_decref(datum);
	aingle_datum_decref(expected_datum);

	// The following should bork if we don't copy the fixed value
	// correctly (since we'll try to free a static string).

	datum = aingle_fixed(schema, "original", 8);
	aingle_fixed_set(datum, "alsothis", 8);
	aingle_datum_decref(datum);

	aingle_schema_decref(schema);
	return 0;
}

int main(void)
{
	aingle_set_allocator(test_allocator, NULL);

	unsigned int i;
	struct aingle_tests {
		char *name;
		aingle_test func;
	} tests[] = {
		{
		"string", test_string}, {
		"bytes", test_bytes}, {
		"int", test_int32}, {
		"long", test_int64}, {
		"float", test_float}, {
		"double", test_double}, {
		"boolean", test_boolean}, {
		"null", test_null}, {
		"record", test_record}, {
		"nested_record", test_nested_record}, {
		"enum", test_enum}, {
		"array", test_array}, {
		"map", test_map}, {
		"fixed", test_fixed}, {
		"union", test_union}
	};

	init_rand();
	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		struct aingle_tests *test = tests + i;
		fprintf(stderr, "**** Running %s tests ****\n", test->name);
		if (test->func() != 0) {
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
