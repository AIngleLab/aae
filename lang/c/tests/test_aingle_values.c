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

/* Test cases for the new aingle_value_t interface */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "aingle.h"
#include "aingle_private.h"

typedef int (*aingle_test) (void);

#ifndef SHOW_ALLOCATIONS
#define SHOW_ALLOCATIONS 0
#endif

/*
 * Use a custom allocator that verifies that the size that we use to
 * free an object matches the size that we use to allocate it.
 */

static void *
test_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
	AINGLE_UNUSED(ud);
	AINGLE_UNUSED(osize);

#if SHOW_ALLOCATIONS
	fprintf(stderr, "alloc(%p, %" PRIsz ", %" PRIsz ") => ", ptr, osize, nsize);
#endif

	if (nsize == 0) {
		size_t  *size = ((size_t *) ptr) - 1;
		if (osize != *size) {
			fprintf(stderr,
#if SHOW_ALLOCATIONS
				"ERROR!\n"
#endif
				"Error freeing %p:\n"
				"Size passed to aingle_free (%" PRIsz ") "
				"doesn't match size passed to "
				"aingle_malloc (%" PRIsz ")\n",
				ptr, osize, *size);
			exit(EXIT_FAILURE);
		}
		free(size);
#if SHOW_ALLOCATIONS
		fprintf(stderr, "NULL\n");
#endif
		return NULL;
	} else {
		size_t  real_size = nsize + sizeof(size_t);
		size_t  *old_size = ptr? ((size_t *) ptr)-1: NULL;
		size_t  *size = (size_t *) realloc(old_size, real_size);
		*size = nsize;
#if SHOW_ALLOCATIONS
		fprintf(stderr, "%p\n", (size+1));
#endif
		return (size + 1);
	}
}

void
init_rand(void)
{
	srand(time(NULL));
}

double
rand_number(double from, double to)
{
	double range = to - from;
	return from + ((double)rand() / (RAND_MAX + 1.0)) * range;
}

int64_t
rand_int64(void)
{
	return (int64_t) rand_number(LONG_MIN, LONG_MAX);
}

int32_t
rand_int32(void)
{
	return (int32_t) rand_number(INT_MIN, INT_MAX);
}

size_t
rand_count(void)
{
	return (size_t) rand_number(0, 100);
}

#define check_(call) \
	do { \
		int _rval = call; \
		if (_rval) { return _rval; } \
	} while (0)

/*
 * Verify that we can't call any of the getters and setters that don't
 * apply to the given value.
 */

static int
_check_invalid_methods(const char *name, aingle_value_t *val)
{
	aingle_type_t  type = aingle_value_get_type(val);

/* For a description on GCC vs Visual Studio 2008 usage of variadic
 * macros see:
 * https://stackoverflow.com/questions/2575864/the-problem-about-different
 * -treatment-to-va-args-when-using-vs-2008-and-gcc
 */
#define expand_args(...) __VA_ARGS__
#define check_bad(method, ...) \
	do { \
          if (!expand_args(aingle_value_##method(__VA_ARGS__))) {  \
			fprintf(stderr, \
				"Shouldn't be able to " #method " a %s\n", \
				name); \
			return EXIT_FAILURE; \
		} \
	} while (0)

	if (type != AINGLE_BOOLEAN) {
		int  dummy = 0;
		check_bad(get_boolean, val, &dummy);
		check_bad(set_boolean, val, dummy);
	}

	if (type != AINGLE_BYTES) {
		const void  *cbuf = NULL;
		void  *buf = NULL;
		size_t  size = 0;
		check_bad(get_bytes, val, &cbuf, &size);
		check_bad(set_bytes, val, buf, size);
	}

	if (type != AINGLE_DOUBLE) {
		double  dummy = 0;
		check_bad(get_double, val, &dummy);
		check_bad(set_double, val, dummy);
	}

	if (type != AINGLE_FLOAT) {
		float  dummy = 0;
		check_bad(get_float, val, &dummy);
		check_bad(set_float, val, dummy);
	}

	if (type != AINGLE_INT32) {
		int32_t  dummy = 0;
		check_bad(get_int, val, &dummy);
		check_bad(set_int, val, dummy);
	}

	if (type != AINGLE_INT64) {
		int64_t  dummy = 0;
		check_bad(get_long, val, &dummy);
		check_bad(set_long, val, dummy);
	}

	if (type != AINGLE_NULL) {
		check_bad(get_null, val);
		check_bad(set_null, val);
	}

	if (type != AINGLE_STRING) {
		const char  *cstr = NULL;
		char  *str = NULL;
		size_t  size = 0;
		check_bad(get_string, val, &cstr, &size);
		check_bad(set_string, val, str);
		check_bad(set_string_len, val, str, size);
	}

	if (type != AINGLE_ENUM) {
		int  dummy = 0;
		check_bad(get_enum, val, &dummy);
		check_bad(set_enum, val, dummy);
	}

	if (type != AINGLE_FIXED) {
		const void  *cbuf = NULL;
		void  *buf = NULL;
		size_t  size = 0;
		check_bad(get_fixed, val, &cbuf, &size);
		check_bad(set_fixed, val, buf, size);
	}

	if (type != AINGLE_ARRAY && type != AINGLE_MAP && type != AINGLE_RECORD) {
		size_t  size = 0;
		check_bad(get_size, val, &size);

		size_t  index = 0;
		aingle_value_t  child;
		const char  *key = NULL;
		check_bad(get_by_index, val, index, &child, &key);
	}

	if (type != AINGLE_MAP && type != AINGLE_RECORD) {
		const char  *key = NULL;
		aingle_value_t  child;
		size_t  index = 0;
		check_bad(get_by_name, val, key, &child, &index);
	}

	if (type != AINGLE_ARRAY) {
		aingle_value_t  child;
		size_t  index;
		check_bad(append, val, &child, &index);
	}

	if (type != AINGLE_MAP) {
		const char  *key = NULL;
		aingle_value_t  child;
		size_t  index = 0;
		int  is_new = 0;
		check_bad(add, val, key, &child, &index, &is_new);
	}

	if (type != AINGLE_UNION) {
		int  discriminant = 0;
		aingle_value_t  branch;
		check_bad(get_discriminant, val, &discriminant);
		check_bad(get_current_branch, val, &branch);
		check_bad(set_branch, val, discriminant, &branch);
	}

#undef check_bad

	return EXIT_SUCCESS;
}

#define check_invalid_methods(name, val) \
	check_(_check_invalid_methods(name, val))

/*
 * Verify that we get the expected type code and schema for a value.
 */

static int
check_type_and_schema(const char *name,
		      aingle_value_t *val,
		      aingle_type_t expected_type,
		      aingle_schema_t expected_schema)
{
	if (aingle_value_get_type(val) != expected_type) {
		aingle_schema_decref(expected_schema);
		fprintf(stderr, "Unexpected type for %s\n", name);
		return EXIT_FAILURE;
	}

	if (!aingle_schema_equal(aingle_value_get_schema(val),
			       expected_schema)) {
		aingle_schema_decref(expected_schema);
		fprintf(stderr, "Unexpected schema for %s\n", name);
		return EXIT_FAILURE;
	}

	aingle_schema_decref(expected_schema);
	return EXIT_SUCCESS;
}

#define try(call, msg) \
	do { \
		if (call) { \
			fprintf(stderr, msg ":\n  %s\n", aingle_strerror()); \
			return EXIT_FAILURE; \
		} \
	} while (0)

static int
_check_write_read(aingle_value_t *val)
{
	static char  buf[4096];

	aingle_reader_t  reader = aingle_reader_memory(buf, sizeof(buf));
	aingle_writer_t  writer = aingle_writer_memory(buf, sizeof(buf));

	if (aingle_value_write(writer, val)) {
		fprintf(stderr, "Unable to write value:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	aingle_writer_dump(writer, stderr);

	size_t size;
	if (aingle_value_sizeof(val, &size)) {
		fprintf(stderr, "Unable to determine size of value:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	if (size != (size_t) aingle_writer_tell(writer)) {
		fprintf(stderr, "Unexpected size of encoded value\n");
		return EXIT_FAILURE;
	}

	aingle_value_t  val_in;
	if (aingle_generic_value_new(val->iface, &val_in)) {
		fprintf(stderr, "Cannot allocate new value instance:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	if (aingle_value_read(reader, &val_in)) {
		fprintf(stderr, "Unable to read value:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	if (!aingle_value_equal(val, &val_in)) {
		fprintf(stderr, "Round-trip values not equal\n");
		exit(EXIT_FAILURE);
	}

	aingle_value_decref(&val_in);
	aingle_reader_free(reader);
	aingle_writer_free(writer);

	return EXIT_SUCCESS;
}

#define check_write_read(val) \
	check_(_check_write_read(val))

static int
_check_hash(aingle_value_t *val1, aingle_value_t *val2)
{
	uint32_t  hash1 = aingle_value_hash(val1);
	uint32_t  hash2 = aingle_value_hash(val2);
	if (hash1 != hash2) {
		fprintf(stderr, "Copied hashed not equal\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

#define check_hash(val1, val2) \
	check_(_check_hash(val1, val2))

static int
_check_copy(aingle_value_t *val)
{
	aingle_value_t  copied_val;
	if (aingle_generic_value_new(val->iface, &copied_val)) {
		fprintf(stderr, "Cannot allocate new value instance:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	if (aingle_value_copy_fast(&copied_val, val)) {
		fprintf(stderr, "Cannot copy value:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	if (!aingle_value_equal(val, &copied_val)) {
		fprintf(stderr, "Copied values not equal\n");
		return EXIT_FAILURE;
	}

	check_hash(val, &copied_val);

	aingle_value_decref(&copied_val);
	return EXIT_SUCCESS;
}

#define check_copy(val) \
	check_(_check_copy(val))

static int
test_boolean(void)
{
	int  rval;

	int  i;
	for (i = 0; i <= 1; i++) {
		aingle_value_t  val;
		try(aingle_generic_boolean_new(&val, i),
		    "Cannot create boolean");
		check(rval, check_type_and_schema
			    ("boolean", &val,
			     AINGLE_BOOLEAN, aingle_schema_boolean()));
		try(aingle_value_reset(&val),
		    "Cannot reset boolean");
		try(aingle_value_set_boolean(&val, i),
		    "Cannot set boolean");

		/* Start with the wrong value to make sure _get does
		 * something. */
		int  actual = (int) 23;
		try(aingle_value_get_boolean(&val, &actual),
		    "Cannot get boolean value");

		if (actual != i) {
			fprintf(stderr, "Unexpected boolean value\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("boolean", &val);
		check_write_read(&val);
		check_copy(&val);
		aingle_value_decref(&val);
	}

	aingle_value_t  val1;
	aingle_value_t  val2;
	try(aingle_generic_boolean_new(&val1, 0),
	    "Cannot create boolean");
	try(aingle_generic_boolean_new(&val2, 1),
	    "Cannot create boolean");
	if (aingle_value_cmp_fast(&val1, &val2) >= 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	if (aingle_value_cmp_fast(&val2, &val1) <= 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	if (aingle_value_cmp_fast(&val1, &val1) != 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	aingle_value_decref(&val1);
	aingle_value_decref(&val2);

	return 0;
}

static int
test_bytes(void)
{
	int  rval;

	char bytes[] = { 0xDE, 0xAD, 0xBE, 0xEF };

	aingle_value_t  val;
	try(aingle_generic_bytes_new(&val, bytes, sizeof(bytes)),
	    "Cannot create bytes");
	check(rval, check_type_and_schema
		    ("bytes", &val,
		     AINGLE_BYTES, aingle_schema_bytes()));
	try(aingle_value_reset(&val),
	    "Cannot reset bytes");
	try(aingle_value_set_bytes(&val, bytes, sizeof(bytes)),
	    "Cannot set bytes");

	const void  *actual_buf = NULL;
	size_t  actual_size = 0;
	try(aingle_value_get_bytes(&val, &actual_buf, &actual_size),
	    "Cannot get bytes value");

	if (actual_size != sizeof(bytes)) {
		fprintf(stderr, "Unexpected bytes size\n");
		return EXIT_FAILURE;
	}

	if (memcmp(actual_buf, bytes, sizeof(bytes)) != 0) {
		fprintf(stderr, "Unexpected bytes contents\n");
		return EXIT_FAILURE;
	}

	aingle_wrapped_buffer_t  wbuf;
	try(aingle_value_grab_bytes(&val, &wbuf),
	    "Cannot grab bytes value");

	if (wbuf.size != sizeof(bytes)) {
		fprintf(stderr, "Unexpected grabbed bytes size\n");
		return EXIT_FAILURE;
	}

	if (memcmp(wbuf.buf, bytes, sizeof(bytes)) != 0) {
		fprintf(stderr, "Unexpected grabbed bytes contents\n");
		return EXIT_FAILURE;
	}

	aingle_wrapped_buffer_free(&wbuf);

	check_invalid_methods("bytes", &val);
	check_write_read(&val);
	check_copy(&val);
	aingle_value_decref(&val);

	aingle_value_t  val1;
	aingle_value_t  val2;
	aingle_value_t  val3;
	try(aingle_generic_bytes_new(&val1, "abcd", 4),
	    "Cannot create bytes");
	try(aingle_generic_bytes_new(&val2, "abcde", 5),
	    "Cannot create bytes");
	try(aingle_generic_bytes_new(&val3, "abce", 4),
	    "Cannot create bytes");
	if (aingle_value_cmp_fast(&val1, &val2) >= 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	if (aingle_value_cmp_fast(&val2, &val1) <= 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	if (aingle_value_cmp_fast(&val1, &val3) >= 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	if (aingle_value_cmp_fast(&val1, &val1) != 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	aingle_value_decref(&val1);
	aingle_value_decref(&val2);
	aingle_value_decref(&val3);

	return 0;
}

static int
test_double(void)
{
	int  rval;

	int  i;
	for (i = 0; i < 100; i++) {
		double  expected = rand_number(-1e10, 1e10);
		aingle_value_t  val;
		try(aingle_generic_double_new(&val, expected),
		    "Cannot create double");
		check(rval, check_type_and_schema
			    ("double", &val,
			     AINGLE_DOUBLE, aingle_schema_double()));
		try(aingle_value_reset(&val),
		    "Cannot reset double");
		try(aingle_value_set_double(&val, expected),
		    "Cannot set double");

		double  actual = 0.0;
		try(aingle_value_get_double(&val, &actual),
		    "Cannot get double value");

		if (actual != expected) {
			fprintf(stderr, "Unexpected double value\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("double", &val);
		check_write_read(&val);
		check_copy(&val);
		aingle_value_decref(&val);
	}
	return 0;
}

static int
test_float(void)
{
	int  rval;

	int  i;
	for (i = 0; i < 100; i++) {
		float  expected = rand_number(-1e10, 1e10);
		aingle_value_t  val;
		try(aingle_generic_float_new(&val, expected),
		    "Cannot create float");
		check(rval, check_type_and_schema
			    ("float", &val,
			     AINGLE_FLOAT, aingle_schema_float()));
		try(aingle_value_reset(&val),
		    "Cannot reset float");
		try(aingle_value_set_float(&val, expected),
		    "Cannot set float");

		float  actual = 0.0f;
		try(aingle_value_get_float(&val, &actual),
		    "Cannot get float value");

		if (actual != expected) {
			fprintf(stderr, "Unexpected float value\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("float", &val);
		check_write_read(&val);
		check_copy(&val);
		aingle_value_decref(&val);
	}
	return 0;
}

static int
test_int(void)
{
	int  rval;

	int  i;
	for (i = 0; i < 100; i++) {
		int32_t  expected = rand_int32();
		aingle_value_t  val;
		try(aingle_generic_int_new(&val, expected),
		    "Cannot create int");
		check(rval, check_type_and_schema
			    ("int", &val,
			     AINGLE_INT32, aingle_schema_int()));
		try(aingle_value_reset(&val),
		    "Cannot reset int");
		try(aingle_value_set_int(&val, expected),
		    "Cannot set int");

		int32_t  actual = 0;
		try(aingle_value_get_int(&val, &actual),
		    "Cannot get int value");

		if (actual != expected) {
			fprintf(stderr, "Unexpected int value\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("int", &val);
		check_write_read(&val);
		check_copy(&val);
		aingle_value_decref(&val);
	}

	aingle_value_t  val1;
	aingle_value_t  val2;
	try(aingle_generic_int_new(&val1, -10),
	    "Cannot create int");
	try(aingle_generic_int_new(&val2, 42),
	    "Cannot create int");
	if (aingle_value_cmp_fast(&val1, &val2) >= 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	if (aingle_value_cmp_fast(&val2, &val1) <= 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	if (aingle_value_cmp_fast(&val1, &val1) != 0) {
		fprintf(stderr, "Incorrect sort order\n");
		return EXIT_FAILURE;
	}
	aingle_value_decref(&val1);
	aingle_value_decref(&val2);

	return 0;
}

static int
test_long(void)
{
	int  rval;

	int  i;
	for (i = 0; i < 100; i++) {
		int64_t  expected = rand_int64();
		aingle_value_t  val;
		try(aingle_generic_long_new(&val, expected),
		    "Cannot create long");
		check(rval, check_type_and_schema
			    ("long", &val,
			     AINGLE_INT64, aingle_schema_long()));
		try(aingle_value_reset(&val),
		    "Cannot reset long");
		try(aingle_value_set_long(&val, expected),
		    "Cannot set long");

		int64_t  actual = 0;
		try(aingle_value_get_long(&val, &actual),
		    "Cannot get long value");

		if (actual != expected) {
			fprintf(stderr, "Unexpected long value\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("long", &val);
		check_write_read(&val);
		check_copy(&val);
		aingle_value_decref(&val);
	}
	return 0;
}

static int
test_null(void)
{
	int  rval;

	aingle_value_t  val;
	try(aingle_generic_null_new(&val),
	    "Cannot create null");
	check(rval, check_type_and_schema
		    ("null", &val,
		     AINGLE_NULL, aingle_schema_null()));
	try(aingle_value_reset(&val),
	    "Cannot reset null");
	try(aingle_value_set_null(&val),
	    "Cannot set null");
	try(aingle_value_get_null(&val),
	    "Cannot get null");

	check_invalid_methods("null", &val);
	check_write_read(&val);
	check_copy(&val);
	aingle_value_decref(&val);
	return 0;
}

static int
test_string(void)
{
	int  rval;

	char *strings[] = {
		"Four score and seven years ago",
		"our father brought forth on this continent",
		"a new nation",
		"conceived in Liberty",
		"and dedicated to the proposition that all men "
			"are created equal."
	};

	unsigned int  i;
	for (i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
		aingle_value_t  val;
		try(aingle_generic_string_new(&val, strings[i]),
		    "Cannot create string");
		check(rval, check_type_and_schema
			    ("string", &val,
			     AINGLE_STRING, aingle_schema_string()));
		try(aingle_value_reset(&val),
		    "Cannot reset string");
		try(aingle_value_set_string_len(&val, "", 0),
		    "Cannot set_len dummy string");

		/* First try a round-trip using set_string */

		try(aingle_value_set_string(&val, strings[i]),
		    "Cannot set string");

		const char  *actual_str = NULL;
		size_t  actual_size = 0;
		try(aingle_value_get_string(&val, &actual_str, &actual_size),
		    "Cannot get string value");

		if (actual_size != strlen(strings[i])+1) {
			fprintf(stderr, "Unexpected string size\n");
			return EXIT_FAILURE;
		}

		if (strcmp(actual_str, strings[i]) != 0) {
			fprintf(stderr, "Unexpected string contents\n");
			return EXIT_FAILURE;
		}

		aingle_wrapped_buffer_t  wbuf;
		try(aingle_value_grab_string(&val, &wbuf),
		    "Cannot grab string value");

		if (wbuf.size != strlen(strings[i])+1) {
			fprintf(stderr, "Unexpected grabbed string size\n");
			return EXIT_FAILURE;
		}

		if (strcmp((const char *) wbuf.buf, strings[i]) != 0) {
			fprintf(stderr, "Unexpected grabbed string contents\n");
			return EXIT_FAILURE;
		}

		aingle_wrapped_buffer_free(&wbuf);

		/* and then again using set_string_len */

		size_t  str_length = strlen(strings[i])+1;
		try(aingle_value_set_string_len(&val, strings[i], str_length),
		    "Cannot set_len string");

		actual_str = NULL;
		actual_size = 0;
		try(aingle_value_get_string(&val, &actual_str, &actual_size),
		    "Cannot get string value");

		if (actual_size != strlen(strings[i])+1) {
			fprintf(stderr, "Unexpected string size\n");
			return EXIT_FAILURE;
		}

		if (strcmp(actual_str, strings[i]) != 0) {
			fprintf(stderr, "Unexpected string contents\n");
			return EXIT_FAILURE;
		}

		try(aingle_value_grab_string(&val, &wbuf),
		    "Cannot grab string value");

		if (wbuf.size != strlen(strings[i])+1) {
			fprintf(stderr, "Unexpected grabbed string size\n");
			return EXIT_FAILURE;
		}

		if (strcmp((const char *) wbuf.buf, strings[i]) != 0) {
			fprintf(stderr, "Unexpected grabbed string contents\n");
			return EXIT_FAILURE;
		}

		aingle_wrapped_buffer_free(&wbuf);

		check_invalid_methods("string", &val);
		check_write_read(&val);
		check_copy(&val);
		aingle_value_decref(&val);
	}

	return 0;
}

static int
test_array(void)
{
	aingle_schema_t  double_schema = aingle_schema_double();
	aingle_schema_t  array_schema = aingle_schema_array(double_schema);

	aingle_value_iface_t  *array_class =
	    aingle_generic_class_from_schema(array_schema);

	int  rval;

	int  i;
	for (i = 0; i < 100; i++) {
		size_t  count = rand_count();

		aingle_value_t  val;
		try(aingle_generic_value_new(array_class, &val),
		    "Cannot create array");
		check(rval, check_type_and_schema
			    ("array", &val, AINGLE_ARRAY,
			     aingle_schema_incref(array_schema)));

		size_t  j;
		for (j = 0; j < count; j++) {
			aingle_value_t  element;
			size_t  new_index;
			try(aingle_value_append(&val, &element, &new_index),
			    "Cannot append to array");
			if (new_index != j) {
				fprintf(stderr, "Unexpected index\n");
				return EXIT_FAILURE;
			}

			double  expected = rand_number(-1e10, 1e10);
			try(aingle_value_set_double(&element, expected),
			    "Cannot set double");
			try(aingle_value_get_by_index(&val, j, &element, NULL),
			    "Cannot get from array");

			double  actual = 0.0;
			try(aingle_value_get_double(&element, &actual),
			    "Cannot get double value");

			if (actual != expected) {
				fprintf(stderr, "Unexpected double value\n");
				return EXIT_FAILURE;
			}
		}

		size_t  actual_size = 0;
		try(aingle_value_get_size(&val, &actual_size),
		    "Cannot get_size array");

		if (actual_size != count) {
			fprintf(stderr, "Unexpected size\n");
			return EXIT_FAILURE;
		}

		check_write_read(&val);
		check_copy(&val);

		try(aingle_value_reset(&val),
		    "Cannot reset array");
		try(aingle_value_get_size(&val, &actual_size),
		    "Cannot get_size empty array");

		if (actual_size != 0) {
			fprintf(stderr, "Unexpected empty size\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("array", &val);
		aingle_value_decref(&val);
	}

	aingle_schema_decref(double_schema);
	aingle_schema_decref(array_schema);
	aingle_value_iface_decref(array_class);
	return 0;
}

static int
test_enum(void)
{
	static const char  SCHEMA_JSON[] =
	"{"
	"  \"type\": \"enum\","
	"  \"name\": \"suits\","
	"  \"symbols\": [\"CLUBS\",\"DIAMONDS\",\"HEARTS\",\"SPADES\"]"
	"}";

	aingle_schema_t  enum_schema = NULL;
	if (aingle_schema_from_json_literal(SCHEMA_JSON, &enum_schema)) {
		fprintf(stderr, "Error parsing schema:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	aingle_value_iface_t  *enum_class =
	    aingle_generic_class_from_schema(enum_schema);

	int  rval;

	int  i;
	for (i = 0; i < 4; i++) {
		int  expected = i;
		aingle_value_t  val;
		try(aingle_generic_value_new(enum_class, &val),
		    "Cannot create enum");
		check(rval, check_type_and_schema
			    ("enum", &val, AINGLE_ENUM,
			     aingle_schema_incref(enum_schema)));
		try(aingle_value_reset(&val),
		    "Cannot reset enum");
		try(aingle_value_set_enum(&val, expected),
		    "Cannot set enum");

		int  actual = -1;
		try(aingle_value_get_enum(&val, &actual),
		    "Cannot get enum value");

		if (actual != expected) {
			fprintf(stderr, "Unexpected enum value\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("enum", &val);
		check_write_read(&val);
		check_copy(&val);
		aingle_value_decref(&val);
	}

	aingle_schema_decref(enum_schema);
	aingle_value_iface_decref(enum_class);
	return 0;
}

static int
test_fixed(void)
{
	static const char  SCHEMA_JSON[] =
	"{"
	"  \"type\": \"fixed\","
	"  \"name\": \"ipv4\","
	"  \"size\": 4"
	"}";

	aingle_schema_t  fixed_schema = NULL;
	if (aingle_schema_from_json_literal(SCHEMA_JSON, &fixed_schema)) {
		fprintf(stderr, "Error parsing schema:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	aingle_value_iface_t  *fixed_class =
	    aingle_generic_class_from_schema(fixed_schema);

	int  rval;

	char fixed[] = { 0xDE, 0xAD, 0xBE, 0xEF };

	aingle_value_t  val;
	try(aingle_generic_value_new(fixed_class, &val),
	    "Cannot create fixed");
	check(rval, check_type_and_schema
		    ("fixed", &val, AINGLE_FIXED,
		     aingle_schema_incref(fixed_schema)));
	try(aingle_value_reset(&val),
	    "Cannot reset fixed");

	/* verify an error on invalid size */
	try(!aingle_value_set_fixed(&val, fixed, 0),
	    "Expected error with invalid size");

	try(aingle_value_set_fixed(&val, fixed, sizeof(fixed)),
	    "Cannot set fixed");

	const void  *actual_buf = NULL;
	size_t  actual_size = 0;
	try(aingle_value_get_fixed(&val, &actual_buf, &actual_size),
	    "Cannot get fixed value");

	if (actual_size != sizeof(fixed)) {
		fprintf(stderr, "Unexpected fixed size\n");
		return EXIT_FAILURE;
	}

	if (memcmp(actual_buf, fixed, sizeof(fixed)) != 0) {
		fprintf(stderr, "Unexpected fixed contents\n");
		return EXIT_FAILURE;
	}

	aingle_wrapped_buffer_t  wbuf;
	try(aingle_value_grab_fixed(&val, &wbuf),
	    "Cannot grab fixed value");

	if (wbuf.size != sizeof(fixed)) {
		fprintf(stderr, "Unexpected grabbed fixed size\n");
		return EXIT_FAILURE;
	}

	if (memcmp(wbuf.buf, fixed, sizeof(fixed)) != 0) {
		fprintf(stderr, "Unexpected grabbed fixed contents\n");
		return EXIT_FAILURE;
	}

	aingle_wrapped_buffer_free(&wbuf);

	check_invalid_methods("fixed", &val);
	check_write_read(&val);
	check_copy(&val);
	aingle_value_decref(&val);
	aingle_schema_decref(fixed_schema);
	aingle_value_iface_decref(fixed_class);
	return 0;
}

static int
test_map(void)
{
	aingle_schema_t  double_schema = aingle_schema_double();
	aingle_schema_t  map_schema = aingle_schema_map(double_schema);

	aingle_value_iface_t  *map_class =
	    aingle_generic_class_from_schema(map_schema);

	int  rval;

	int  i;
	for (i = 0; i < 100; i++) {
		size_t  count = rand_count();

		aingle_value_t  val;
		try(aingle_generic_value_new(map_class, &val),
		    "Cannot create map");
		check(rval, check_type_and_schema
			    ("map", &val, AINGLE_MAP,
			     aingle_schema_incref(map_schema)));

		size_t  j;
		for (j = 0; j < count; j++) {
			aingle_value_t  element;
			size_t  new_index;
			int  is_new = 0;

			char  key[64];
			snprintf(key, 64, "%" PRIsz, j);

			try(aingle_value_add(&val, key,
					   &element, &new_index, &is_new),
			    "Cannot add to map");

			if (new_index != j) {
				fprintf(stderr, "Unexpected index\n");
				return EXIT_FAILURE;
			}

			if (!is_new) {
				fprintf(stderr, "Expected new element\n");
				return EXIT_FAILURE;
			}

			double  expected = rand_number(-1e10, 1e10);
			try(aingle_value_set_double(&element, expected),
			    "Cannot set double");
			try(aingle_value_add(&val, key,
					   &element, &new_index, &is_new),
			    "Cannot re-add to map");

			if (is_new) {
				fprintf(stderr, "Expected non-new element\n");
				return EXIT_FAILURE;
			}

			const char  *actual_key = NULL;
			try(aingle_value_get_by_index(&val, j, &element,
						    &actual_key),
			    "Cannot get from map");

			if (strcmp(actual_key, key) != 0) {
				fprintf(stderr, "Unexpected key\n");
				return EXIT_FAILURE;
			}

			double  actual = 0.0;
			try(aingle_value_get_double(&element, &actual),
			    "Cannot get double value");

			if (actual != expected) {
				fprintf(stderr, "Unexpected double value\n");
				return EXIT_FAILURE;
			}
		}

		size_t  actual_size = 0;
		try(aingle_value_get_size(&val, &actual_size),
		    "Cannot get_size map");

		if (actual_size != count) {
			fprintf(stderr, "Unexpected size\n");
			return EXIT_FAILURE;
		}

		/*
		 * Create a reversed copy of the map to ensure that the
		 * element ordering doesn't affect the hash value.
		 */

		aingle_value_t  reversed;
		try(aingle_generic_value_new(map_class, &reversed),
		    "Cannot create map");

		for (j = count; j-- > 0; ) {
			aingle_value_t  element;
			const char  *key = NULL;
			double  element_value = 0.0;
			try(aingle_value_get_by_index(&val, j, &element, &key),
			    "Cannot get from map");
			try(aingle_value_get_double(&element, &element_value),
			    "Cannot get double value");

			try(aingle_value_add(&reversed, key, &element, NULL, NULL),
			    "Cannot add to map");
			try(aingle_value_set_double(&element, element_value),
			    "Cannot set double");
		}

		check_hash(&val, &reversed);
		if (!aingle_value_equal(&val, &reversed)) {
			fprintf(stderr, "Reversed values not equal\n");
			return EXIT_FAILURE;
		}

		/* Final tests and cleanup */

		check_write_read(&val);
		check_copy(&val);

		try(aingle_value_reset(&val),
		    "Cannot reset map");
		try(aingle_value_get_size(&val, &actual_size),
		    "Cannot get_size empty map");

		if (actual_size != 0) {
			fprintf(stderr, "Unexpected empty size\n");
			return EXIT_FAILURE;
		}

		check_invalid_methods("map", &val);
		aingle_value_decref(&val);
		aingle_value_decref(&reversed);
	}

	aingle_schema_decref(double_schema);
	aingle_schema_decref(map_schema);
	aingle_value_iface_decref(map_class);
	return 0;
}

static int
test_record(void)
{
	static const char  SCHEMA_JSON[] =
	"{"
	"  \"type\": \"record\","
	"  \"name\": \"test\","
	"  \"fields\": ["
	"    { \"name\": \"b\", \"type\": \"boolean\" },"
	"    { \"name\": \"i\", \"type\": \"int\" },"
	"    { \"name\": \"s\", \"type\": \"string\" },"
	"    { \"name\": \"ds\", \"type\": "
	"      { \"type\": \"array\", \"items\": \"double\" } },"
	"    { \"name\": \"sub\", \"type\": "
	"      {"
	"        \"type\": \"record\","
	"        \"name\": \"subtest\","
	"        \"fields\": ["
	"          { \"name\": \"f\", \"type\": \"float\" },"
	"          { \"name\": \"l\", \"type\": \"long\" }"
	"        ]"
	"      }"
	"    },"
	"    { \"name\": \"nested\", \"type\": [\"null\", \"test\"] }"
	"  ]"
	"}";

	aingle_schema_t  record_schema = NULL;
	if (aingle_schema_from_json_literal(SCHEMA_JSON, &record_schema)) {
		fprintf(stderr, "Error parsing schema:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	aingle_value_iface_t  *record_class =
	    aingle_generic_class_from_schema(record_schema);

	int  rval;

	aingle_value_t  val;
	try(aingle_generic_value_new(record_class, &val),
	    "Cannot create record");
	check(rval, check_type_and_schema
		    ("record", &val, AINGLE_RECORD,
		     aingle_schema_incref(record_schema)));

	size_t  field_count;
	try(aingle_value_get_size(&val, &field_count),
	    "Cannot get field count");
	if (field_count != 6) {
		fprintf(stderr, "Unexpected field count\n");
		return EXIT_FAILURE;
	}

	/* Assign to each field */
	aingle_value_t  field;
	aingle_value_t  element;
	aingle_value_t  subfield;
	aingle_value_t  branch;
	const char  *name;
	size_t  index;

	try(aingle_value_get_by_index(&val, 0, &field, NULL),
	    "Cannot get field 0");
	try(aingle_value_set_boolean(&field, 1),
	    "Cannot set field 0");

	try(aingle_value_get_by_index(&val, 1, &field, &name),
	    "Cannot get field 1");
	try(aingle_value_set_int(&field, 42),
	    "Cannot set field 1");
	if (strcmp(name, "i") != 0) {
		fprintf(stderr, "Unexpected name for field 1: %s\n", name);
		return EXIT_FAILURE;
	}

	try(aingle_value_get_by_index(&val, 2, &field, NULL),
	    "Cannot get field 2");
	try(aingle_value_set_string(&field, "Hello world!"),
	    "Cannot set field 2");

	try(aingle_value_get_by_name(&val, "i", &field, &index),
	    "Cannot get \"i\" field");
	if (index != 1) {
		fprintf(stderr, "Unexpected index for \"i\" field: %" PRIsz "\n", index);
		return EXIT_FAILURE;
	}

	try(aingle_value_get_by_index(&val, 3, &field, NULL),
	    "Cannot get field 3");
	try(aingle_value_append(&field, &element, NULL),
	    "Cannot append to field 3");
	try(aingle_value_set_double(&element, 10.0),
	    "Cannot set field 3, element 0");

	try(aingle_value_get_by_index(&val, 4, &field, NULL),
	    "Cannot get field 4");

	try(aingle_value_get_by_index(&field, 0, &subfield, NULL),
	    "Cannot get field 4, subfield 0");
	try(aingle_value_set_float(&subfield, 5.0f),
	    "Cannot set field 4, subfield 0");

	try(aingle_value_get_by_index(&field, 1, &subfield, NULL),
	    "Cannot get field 4, subfield 1");
	try(aingle_value_set_long(&subfield, 10000),
	    "Cannot set field 4, subfield 1");

	try(aingle_value_get_by_index(&val, 5, &field, NULL),
	    "Cannot get field 5");
	try(aingle_value_set_branch(&field, 0, &branch),
	    "Cannot select null branch");

	check_write_read(&val);
	check_copy(&val);

	/* Reset and verify that the fields are empty again */
	try(aingle_value_reset(&val),
	    "Cannot reset record");

	int  bval;
	try(aingle_value_get_by_index(&val, 0, &field, NULL),
	    "Cannot get field 0");
	try(aingle_value_get_boolean(&field, &bval),
	    "Cannot get field 0 value");
	if (bval) {
		fprintf(stderr, "Unexpected value for field 0\n");
		return EXIT_FAILURE;
	}

	size_t  count;
	try(aingle_value_get_by_index(&val, 3, &field, NULL),
	    "Cannot get field 3");
	try(aingle_value_get_size(&field, &count),
	    "Cannot get field 3 size");
	if (count != 0) {
		fprintf(stderr, "Unexpected size for field 3\n");
		return EXIT_FAILURE;
	}

	check_invalid_methods("record", &val);
	aingle_value_decref(&val);
	aingle_value_iface_decref(record_class);
	aingle_schema_decref(record_schema);
	return EXIT_SUCCESS;
}

static int
test_union(void)
{
	static const char  SCHEMA_JSON[] =
	"["
	"  \"null\","
	"  \"int\","
	"  \"double\","
	"  \"bytes\""
	"]";

	aingle_schema_t  union_schema = NULL;
	if (aingle_schema_from_json_literal(SCHEMA_JSON, &union_schema)) {
		fprintf(stderr, "Error parsing schema:\n  %s\n",
			aingle_strerror());
		return EXIT_FAILURE;
	}

	aingle_value_iface_t  *union_class =
	    aingle_generic_class_from_schema(union_schema);

	int  rval;

	aingle_value_t  val;
	try(aingle_generic_value_new(union_class, &val),
	    "Cannot create union");
	check(rval, check_type_and_schema
		    ("union", &val, AINGLE_UNION,
		     aingle_schema_incref(union_schema)));

	int discriminant = 0;
	try(aingle_value_get_discriminant(&val, &discriminant),
	    "Cannot get union discriminant");

	if (discriminant != -1) {
		fprintf(stderr, "Unexpected union discriminant\n");
		return EXIT_FAILURE;
	}

	aingle_value_t  branch;
	try(!aingle_value_get_current_branch(&val, &branch),
	    "Expected error getting empty current branch");

	try(aingle_value_set_branch(&val, 0, &branch),
	    "Cannot select null branch");
	try(aingle_value_set_null(&branch),
	    "Cannot set null branch value");

	try(aingle_value_set_branch(&val, 1, &branch),
	    "Cannot select int branch");
	try(aingle_value_set_int(&branch, 42),
	    "Cannot set int branch value");

	try(aingle_value_set_branch(&val, 1, &branch),
	    "Cannot select int branch");
	try(aingle_value_set_int(&branch, 10),
	    "Cannot set int branch value");

	try(aingle_value_set_branch(&val, 2, &branch),
	    "Cannot select double branch");
	try(aingle_value_set_double(&branch, 10.0),
	    "Cannot set double branch value");

	char bytes[] = { 0xDE, 0xAD, 0xBE, 0xEF };
	try(aingle_value_set_branch(&val, 3, &branch),
	    "Cannot select bytes branch");
	try(aingle_value_set_bytes(&branch, bytes, sizeof(bytes)),
	    "Cannot set bytes branch value");

	check_invalid_methods("union", &val);
	check_write_read(&val);
	check_copy(&val);
	aingle_value_decref(&val);

	aingle_schema_decref(union_schema);
	aingle_value_iface_decref(union_class);
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
		{ "boolean", test_boolean },
		{ "bytes", test_bytes },
		{ "double", test_double },
		{ "float", test_float },
		{ "int", test_int },
		{ "long", test_long },
		{ "null", test_null },
		{ "string", test_string },
		{ "array", test_array },
		{ "enum", test_enum },
		{ "fixed", test_fixed },
		{ "map", test_map },
		{ "record", test_record },
		{ "union", test_union }
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
