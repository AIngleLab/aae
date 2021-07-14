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
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aingle.h"
#include "aingle_private.h"


/* The compression codec to use. */
static const char  *codec = "null";

/* The block size to use. */
static size_t  block_size = 0;

/*-- PROCESSING A FILE --*/

static void
process_file(const char *in_filename, const char *out_filename)
{
	aingle_file_reader_t  reader;
	aingle_file_writer_t  writer;

	if (in_filename == NULL) {
		if (aingle_file_reader_fp(stdin, "<stdin>", 0, &reader)) {
			fprintf(stderr, "Error opening <stdin>:\n  %s\n",
				aingle_strerror());
			exit(1);
		}
	} else {
		if (aingle_file_reader(in_filename, &reader)) {
			fprintf(stderr, "Error opening %s:\n  %s\n",
				in_filename, aingle_strerror());
			exit(1);
		}
	}

	aingle_schema_t  wschema;
	aingle_value_iface_t  *iface;
	aingle_value_t  value;
	int rval;

	wschema = aingle_file_reader_get_writer_schema(reader);
	iface = aingle_generic_class_from_schema(wschema);
	aingle_generic_value_new(iface, &value);

	if (aingle_file_writer_create_with_codec
	    (out_filename, wschema, &writer, codec, block_size)) {
		fprintf(stderr, "Error creating %s:\n  %s\n",
			out_filename, aingle_strerror());
		exit(1);
	}

	while ((rval = aingle_file_reader_read_value(reader, &value)) == 0) {
		if (aingle_file_writer_append_value(writer, &value)) {
			fprintf(stderr, "Error writing to %s:\n  %s\n",
				out_filename, aingle_strerror());
			exit(1);
		}
		aingle_value_reset(&value);
	}

	if (rval != EOF) {
		fprintf(stderr, "Error reading value: %s", aingle_strerror());
	}

	aingle_file_reader_close(reader);
	aingle_file_writer_close(writer);
	aingle_value_decref(&value);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(wschema);
}


/*-- MAIN PROGRAM --*/

static struct option longopts[] = {
	{ "block-size", required_argument, NULL, 'b' },
	{ "codec", required_argument, NULL, 'c' },
	{ NULL, 0, NULL, 0 }
};

static void usage(void)
{
	fprintf(stderr,
		"Usage: ainglemod [--codec=<compression codec>]\n"
		"               [--block-size=<block size>]\n"
		"               [<input aingle file>]\n"
		"                <output aingle file>\n");
}

static void
parse_block_size(const char *optarg)
{
	unsigned long  ul;
	char  *end;

	ul = strtoul(optarg, &end, 10);
	if ((ul == 0 && end == optarg) ||
	    (ul == ULONG_MAX && errno == ERANGE)) {
		fprintf(stderr, "Invalid block size: %s\n\n", optarg);
		usage();
		exit(1);
	}
	block_size = ul;
}


int main(int argc, char **argv)
{
	char  *in_filename;
	char  *out_filename;

	int  ch;
	while ((ch = getopt_long(argc, argv, "b:c:", longopts, NULL)) != -1) {
		switch (ch) {
			case 'b':
				parse_block_size(optarg);
				break;

			case 'c':
				codec = optarg;
				break;

			default:
				usage();
				exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 2) {
		in_filename = argv[0];
		out_filename = argv[1];
	} else if (argc == 1) {
		in_filename = NULL;
		out_filename = argv[0];
	} else {
		fprintf(stderr, "Can't read from multiple input files.\n");
		usage();
		exit(1);
	}

	/* Process the data file */
	process_file(in_filename, out_filename);
	return 0;
}
