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
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <unistd.h>
#endif

#include "aingle.h"

int process_file(const char *in_filename, const char *out_filename)
{
	aingle_file_reader_t  reader;
	aingle_file_writer_t  writer;

	if (in_filename == NULL) {
		if (aingle_file_reader_fp(stdin, "<stdin>", 0, &reader)) {
			fprintf(stderr, "Error opening <stdin>:\n  %s\n",
				aingle_strerror());
			return 1;
		}
	} else {
		if (aingle_file_reader(in_filename, &reader)) {
			fprintf(stderr, "Error opening %s:\n  %s\n",
				in_filename, aingle_strerror());
			return 1;
		}
	}

	aingle_schema_t  wschema;
	wschema = aingle_file_reader_get_writer_schema(reader);

	/* Check that the reader schema is the same as the writer schema */
	{
		aingle_schema_t oschema;
		aingle_file_reader_t oreader;

		if (aingle_file_reader(out_filename, &oreader)) {
			fprintf(stderr, "Error opening %s:\n   %s\n",
					out_filename, aingle_strerror());
			aingle_file_reader_close(reader);
			return 1;
		}

		oschema = aingle_file_reader_get_writer_schema(oreader);

		if (aingle_schema_equal(oschema, wschema) == 0) {
			fprintf(stderr, "Error: reader and writer schema are not equal.\n");
			aingle_file_reader_close(oreader);
			aingle_file_reader_close(reader);
			return 1;
		}

		aingle_file_reader_close(oreader);
		aingle_schema_decref(oschema);
	}

	if (aingle_file_writer_open(out_filename, &writer)) {
		fprintf(stderr, "Error opening %s:\n   %s\n",
				out_filename, aingle_strerror());
		aingle_file_reader_close(reader);
		return 1;
	}

	aingle_value_iface_t  *iface;
	aingle_value_t  value;

	iface = aingle_generic_class_from_schema(wschema);
	aingle_generic_value_new(iface, &value);

	while (aingle_file_reader_read_value(reader, &value) == 0) {
		if (aingle_file_writer_append_value(writer, &value)) {
			fprintf(stderr, "Error writing to %s:\n  %s\n",
				out_filename, aingle_strerror());
			return 1;
		}
		aingle_value_reset(&value);
	}

	aingle_file_reader_close(reader);
	aingle_file_writer_close(writer);
	aingle_value_decref(&value);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(wschema);

	return 0;
}

static void usage(void)
{
	fprintf(stderr,
		"Usage: aingleappend [<input aingle file>] <output aingle file>\n");
}

static int check_filenames(const char *in_filename, const char *out_filename)
{
	if (in_filename == NULL) {
		return 0;
	}

	struct stat in_stat;
	struct stat out_stat;

	if (stat(in_filename, &in_stat) == -1) {
		fprintf(stderr, "stat error on %s: %s\n", in_filename, strerror(errno));
		return 2;
	}

	if (stat(out_filename, &out_stat) == -1) {
		fprintf(stderr, "stat error on %s: %s\n", out_filename, strerror(errno));
		return 2;
	}

	if (in_stat.st_dev == out_stat.st_dev && in_stat.st_ino == out_stat.st_ino) {
		return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	char *in_filename;
	char *out_filename;

	argc--;
	argv++;

	if (argc == 2) {
		in_filename = argv[0];
		out_filename = argv[1];
	} else if (argc == 1) {
		in_filename = NULL;
		out_filename = argv[0];
	} else {
		fprintf(stderr, "Not enough arguments\n\n");
		usage();
		exit(1);
	}

	int ret = check_filenames(in_filename, out_filename);

	if (ret == 1) {
		fprintf(stderr, "Files are the same.\n");
	}

	if (ret > 0) {
		exit(1);
	}

	exit(process_file(in_filename, out_filename));
}
