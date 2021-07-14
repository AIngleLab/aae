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

int main(void)
{
	aingle_file_reader_t  reader;

	/* First open the file with the explicit codec. */
	check_exit(aingle_file_reader("aingle-1279-codec.aingle", &reader));
	check_exit(aingle_file_reader_close(reader));


	/* Then the file with no codec. */
	check_exit(aingle_file_reader("aingle-1279-no-codec.aingle", &reader));
	check_exit(aingle_file_reader_close(reader));

	/* Clean up and exit */
	exit(EXIT_SUCCESS);
}
