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

#include "aingle.h"
#include "aingle_private.h"

#define try(call, msg) \
	do { \
		if (call) { \
			fprintf(stderr, msg ":\n  %s\n", aingle_strerror()); \
			return EXIT_FAILURE; \
		} \
	} while (0)

int
main(int argc, char **argv)
{
	AINGLE_UNUSED(argc);
	AINGLE_UNUSED(argv);

	aingle_value_t  v1;
	aingle_value_t  v2;

	try(aingle_generic_string_new(&v1, "test string a"),
	    "Cannot create string value");
	try(aingle_generic_string_new(&v2, "test string b"),
	    "Cannot create string value");

	if (aingle_value_equal(&v1, &v2)) {
		fprintf(stderr, "Unexpected aingle_value_equal\n");
		return EXIT_FAILURE;
	}

	if (aingle_value_equal_fast(&v1, &v2)) {
		fprintf(stderr, "Unexpected aingle_value_equal_fast\n");
		return EXIT_FAILURE;
	}

	if (aingle_value_cmp(&v1, &v2) >= 0) {
		fprintf(stderr, "Unexpected aingle_value_cmp\n");
		return EXIT_FAILURE;
	}

	if (aingle_value_cmp_fast(&v1, &v2) >= 0) {
		fprintf(stderr, "Unexpected aingle_value_cmp_fast\n");
		return EXIT_FAILURE;
	}

	aingle_value_decref(&v1);
	aingle_value_decref(&v2);
	return 0;
}
