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
#include <string.h>
#include <aingle.h>

/* To see the AINGLE-766 memory leak, run this test program through
 * valgrind.  The specific valgrind commandline to use from the
 * aingle-trunk/lang/c/tests directory is:
 *    valgrind -v --track-origins=yes --leak-check=full
 *          --show-reachable = yes ../build/tests/test_aingle_766
 */
int main(int argc, char **argv)
{
	const char  *json =
		"{"
		"  \"type\": \"record\","
		"  \"name\": \"list\","
		"  \"fields\": ["
		"    { \"name\": \"x\", \"type\": \"int\" },"
		"    { \"name\": \"y\", \"type\": \"int\" },"
		"    { \"name\": \"next\", \"type\": [\"null\",\"list\"]},"
		"    { \"name\": \"arraylist\", \"type\": { \"type\":\"array\", \"items\": \"list\" } }"
		"  ]"
		"}";

	int rval;
	aingle_schema_t schema = NULL;
	aingle_schema_error_t error;

	(void) argc;
	(void) argv;

	rval = aingle_schema_from_json(json, strlen(json), &schema, &error);
	if ( rval )
	{
		printf("Failed to read schema from JSON.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("Successfully read schema from JSON.\n");
	}

#define TEST_AINGLE_1167 (1)
       #if TEST_AINGLE_1167
	{
		aingle_schema_t schema_copy = NULL;
		schema_copy = aingle_schema_copy( schema );
		if ( ! aingle_schema_equal(schema, schema_copy) )
		{
			printf("Failed aingle_schema_equal(schema, schema_copy)\n");
			exit(EXIT_FAILURE);
		}
		aingle_schema_decref(schema_copy);
	}
       #endif

	aingle_schema_decref(schema);
	return 0;
}
