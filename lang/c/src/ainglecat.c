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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aingle.h"
#include "aingle_private.h"


/*-- PROCESSING A FILE --*/

static void
process_file(const char *filename)
{
	aingle_file_reader_t  reader;
	FILE *fp;
	int  should_close;

	if (filename == NULL) {
		fp = stdin;
		filename = "<stdin>";
		should_close = 0;
	} else {
		fp = fopen(filename, "rb");
		should_close = 1;

		if (fp == NULL) {
			fprintf(stderr, "Error opening %s:\n  %s\n",
				filename, strerror(errno));
			exit(1);
		}
	}

	if (aingle_file_reader_fp(fp, filename, 0, &reader)) {
		fprintf(stderr, "Error opening %s:\n  %s\n",
			filename, aingle_strerror());
		if (should_close) {
			fclose(fp);
		}
		exit(1);
	}

	aingle_schema_t  wschema;
	aingle_value_iface_t  *iface;
	aingle_value_t  value;

	wschema = aingle_file_reader_get_writer_schema(reader);
	iface = aingle_generic_class_from_schema(wschema);
	aingle_generic_value_new(iface, &value);

	int rval;

	while ((rval = aingle_file_reader_read_value(reader, &value)) == 0) {
		char  *json;

		if (aingle_value_to_json(&value, 1, &json)) {
			fprintf(stderr, "Error converting value to JSON: %s\n",
				aingle_strerror());
		} else {
			printf("%s\n", json);
			free(json);
		}

		aingle_value_reset(&value);
	}

	// If it was not an EOF that caused it to fail,
	// print the error.
	if (rval != EOF) {
		fprintf(stderr, "Error: %s\n", aingle_strerror());
	}

	aingle_file_reader_close(reader);
	aingle_value_decref(&value);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(wschema);

	if (should_close) {
		fclose(fp);
	}
}


/*-- MAIN PROGRAM --*/

static void usage(void)
{
	fprintf(stderr,
		"Usage: ainglecat <aingle data file>\n");
}


int main(int argc, char **argv)
{
	char  *data_filename;

	if (argc == 2) {
		data_filename = argv[1];
	} else if (argc == 1) {
		data_filename = NULL;
	} else {
		fprintf(stderr, "Can't read from multiple input files.\n");
		usage();
		exit(1);
	}

	/* Process the data file */
	process_file(data_filename);
	return 0;
}
