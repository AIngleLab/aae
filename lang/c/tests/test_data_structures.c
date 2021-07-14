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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aingle_private.h"
#include "aingle/data.h"

static int  result = EXIT_SUCCESS;

typedef int (*aingle_test) (void);


static int
test_array(void)
{
	aingle_raw_array_t  array;
	long  *element;

	/* Test once on a fresh array */

	aingle_raw_array_init(&array, sizeof(long));
	element = (long *) aingle_raw_array_append(&array);
	*element = 1;
	element = (long *) aingle_raw_array_append(&array);
	*element = 3;

	if (aingle_raw_array_size(&array) != 2) {
		fprintf(stderr, "Incorrect array size: got %lu, expected %lu.\n",
			(unsigned long) aingle_raw_array_size(&array),
			(unsigned long) 2);
		return EXIT_FAILURE;
	}

	if (aingle_raw_array_get(&array, long, 0) != 1) {
		fprintf(stderr, "Unexpected array element %u: got %ld, expected %ld.\n",
			(unsigned int) 0, aingle_raw_array_get(&array, long, 0),
			(long) 1);
		return EXIT_FAILURE;
	}

	/* And test again after clearing the array */

	aingle_raw_array_clear(&array);
	element = (long *) aingle_raw_array_append(&array);
	*element = 1;
	element = (long *) aingle_raw_array_append(&array);
	*element = 3;

	if (aingle_raw_array_size(&array) != 2) {
		fprintf(stderr, "Incorrect array size: got %" PRIsz ", expected %" PRIsz ".\n",
			(size_t) aingle_raw_array_size(&array),
			(size_t) 2);
		return EXIT_FAILURE;
	}

	if (aingle_raw_array_get(&array, long, 0) != 1) {
		fprintf(stderr, "Unexpected array element %u: got %ld, expected %ld.\n",
			(unsigned int) 0, aingle_raw_array_get(&array, long, 0),
			(long) 1);
		return EXIT_FAILURE;
	}

	aingle_raw_array_done(&array);
	return EXIT_SUCCESS;
}


static int
test_map(void)
{
	aingle_raw_map_t  map;
	long  *element;
	size_t  index;

	/* Test once on a fresh map */

	aingle_raw_map_init(&map, sizeof(long));
	aingle_raw_map_get_or_create(&map, "x", (void **) &element, NULL);
	*element = 1;
	aingle_raw_map_get_or_create(&map, "y", (void **) &element, NULL);
	*element = 3;

	if (aingle_raw_map_size(&map) != 2) {
		fprintf(stderr, "Incorrect map size: got %" PRIsz ", expected %" PRIsz ".\n",
			(size_t) aingle_raw_map_size(&map),
			(size_t) 2);
		return EXIT_FAILURE;
	}

	if (aingle_raw_map_get_by_index(&map, long, 0) != 1) {
		fprintf(stderr, "Unexpected map element %u: got %ld, expected %ld.\n",
			(unsigned int) 0,
			aingle_raw_map_get_by_index(&map, long, 0),
			(long) 1);
		return EXIT_FAILURE;
	}

	if (strcmp(aingle_raw_map_get_key(&map, 0), "x") != 0) {
		fprintf(stderr, "Unexpected key for map element 0: "
			"got \"%s\", expected \"%s\".\n",
			aingle_raw_map_get_key(&map, 0), "x");
		return EXIT_FAILURE;
	}

	element = (long *) aingle_raw_map_get(&map, "y", &index);
	if (index != 1) {
		fprintf(stderr, "Unexpected index for map element \"%s\": "
			"got %" PRIsz ", expected %u.\n",
			"y", index, 1);
		return EXIT_FAILURE;
	}

	if (*element != 3) {
		fprintf(stderr, "Unexpected map element %s: got %ld, expected %ld.\n",
			"y",
			*element, (long) 3);
		return EXIT_FAILURE;
	}

	/* And test again after clearing the map */

	aingle_raw_map_clear(&map);
	aingle_raw_map_get_or_create(&map, "x", (void **) &element, NULL);
	*element = 1;
	aingle_raw_map_get_or_create(&map, "y", (void **) &element, NULL);
	*element = 3;

	if (aingle_raw_map_size(&map) != 2) {
		fprintf(stderr, "Incorrect map size: got %" PRIsz ", expected %" PRIsz ".\n",
			(size_t) aingle_raw_map_size(&map),
			(size_t) 2);
		return EXIT_FAILURE;
	}

	if (aingle_raw_map_get_by_index(&map, long, 0) != 1) {
		fprintf(stderr, "Unexpected map element %u: got %ld, expected %ld.\n",
			(unsigned int) 0,
			aingle_raw_map_get_by_index(&map, long, 0),
			(long) 1);
		return EXIT_FAILURE;
	}

	element = (long *) aingle_raw_map_get(&map, "y", &index);
	if (index != 1) {
		fprintf(stderr, "Unexpected index for map element \"%s\": "
			"got %" PRIsz ", expected %u.\n",
			"y", index, 1);
		return EXIT_FAILURE;
	}

	if (*element != 3) {
		fprintf(stderr, "Unexpected map element %s: got %ld, expected %ld.\n",
			"y",
			*element, (long) 3);
		return EXIT_FAILURE;
	}

	aingle_raw_map_done(&map);
	return EXIT_SUCCESS;
}


static int
test_string(void)
{
	aingle_raw_string_t  str;

	aingle_raw_string_init(&str);

	aingle_raw_string_set(&str, "a");
	aingle_raw_string_set(&str, "abcdefgh");
	aingle_raw_string_set(&str, "abcd");

	if (aingle_raw_string_length(&str) != 5) {
		fprintf(stderr, "Incorrect string size: got %" PRIsz ", expected %" PRIsz ".\n",
			(size_t) aingle_raw_string_length(&str),
			(size_t) 5);
		return EXIT_FAILURE;
	}

	if (strcmp((const char *) str.wrapped.buf, "abcd") != 0) {
		fprintf(stderr, "Incorrect string contents: "
				"got \"%s\", expected \"%s\".\n",
			(char *) aingle_raw_string_get(&str),
			"abcd");
		return EXIT_FAILURE;
	}

	aingle_wrapped_buffer_t  wbuf;
	aingle_wrapped_buffer_new_string(&wbuf, "abcd");
	aingle_raw_string_give(&str, &wbuf);

	if (aingle_raw_string_length(&str) != 5) {
		fprintf(stderr, "Incorrect string size: got %" PRIsz ", expected %" PRIsz ".\n",
			(size_t) aingle_raw_string_length(&str),
			(size_t) 5);
		return EXIT_FAILURE;
	}

	if (strcmp((const char *) str.wrapped.buf, "abcd") != 0) {
		fprintf(stderr, "Incorrect string contents: "
				"got \"%s\", expected \"%s\".\n",
			(char *) aingle_raw_string_get(&str),
			"abcd");
		return EXIT_FAILURE;
	}

	aingle_raw_string_t  str2;
	aingle_raw_string_init(&str2);
	aingle_raw_string_set(&str2, "abcd");

	if (!aingle_raw_string_equals(&str, &str2)) {
		fprintf(stderr, "Strings should be equal.\n");
		return EXIT_FAILURE;
	}

	aingle_raw_string_done(&str);
	aingle_raw_string_done(&str2);
	return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
	AINGLE_UNUSED(argc);
	AINGLE_UNUSED(argv);

	unsigned int i;
	struct aingle_tests {
		char *name;
		aingle_test func;
	} tests[] = {
		{ "array", test_array },
		{ "map", test_map },
		{ "string", test_string }
	};

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		struct aingle_tests *test = tests + i;
		fprintf(stderr, "**** Running %s tests ****\n", test->name);
		if (test->func() != 0) {
			result = EXIT_FAILURE;
		}
	}
	return result;
}
