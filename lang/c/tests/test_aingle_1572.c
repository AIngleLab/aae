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
#include <sys/stat.h>
#include "aingle.h"

#define BUFFER_LEN 4096

#define HEADER_LEN 116
#define SYNC_LEN 16
#define BLOCKINFO_LEN 4
#define RECORD_LEN 1

static const int NUM_RECORDS1 = (BUFFER_LEN - HEADER_LEN - 2 * SYNC_LEN - BLOCKINFO_LEN) / RECORD_LEN;
static const int NUM_RECORDS2 = (BUFFER_LEN - SYNC_LEN - BLOCKINFO_LEN) / RECORD_LEN;

static const char  PERSON_SCHEMA[] =
	"{"
	"    \"type\":\"record\","
	"    \"name\":\"Person\","
	"    \"fields\": ["
	"        {\"name\": \"ab\", \"type\": \"int\"}"
	"    ]"
	"}";

static const char *filename = "aingle_file.dat";

static int read_data() {
	int rval;
	int records_read = 0;

	aingle_file_reader_t reader;
	aingle_value_iface_t *iface;
	aingle_value_t value;

	fprintf(stderr, "\nReading...\n");

	aingle_file_reader(filename, &reader);
	aingle_schema_t schema = aingle_file_reader_get_writer_schema(reader);

	iface = aingle_generic_class_from_schema(schema);
	aingle_generic_value_new(iface, &value);

	while ((rval = aingle_file_reader_read_value(reader, &value)) == 0) {
		records_read++;
		aingle_value_reset(&value);
	}

	aingle_value_decref(&value);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(schema);
	aingle_file_reader_close(reader);

	fprintf(stderr, "wanted %d records, read %d records.\n", NUM_RECORDS1 + NUM_RECORDS2, records_read);

	if (rval != EOF || records_read != (NUM_RECORDS1 + NUM_RECORDS2)) {
		fprintf(stderr, "Error: %s\n", aingle_strerror());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static off_t fsize(const char *filename) {
	struct stat st;

	if (stat(filename, &st) == 0) {
		return st.st_size;
	}

	return -1;
}

static int write_data() {
	int  i;
	aingle_schema_t schema;
	aingle_schema_error_t error;
	aingle_file_writer_t writer;
	aingle_value_iface_t *iface;
	aingle_value_t value;
	aingle_value_t field;

	fprintf(stderr, "\nWriting...\n");

	if (aingle_schema_from_json(PERSON_SCHEMA, 0, &schema, &error)) {
		fprintf(stderr, "Unable to parse schema\n");
		return EXIT_FAILURE;
	}

	iface = aingle_generic_class_from_schema(schema);
	aingle_generic_value_new(iface, &value);

	if (aingle_file_writer_create(filename, schema, &writer)) {
		fprintf(stderr, "There was an error creating file: %s\n", aingle_strerror());
		return EXIT_FAILURE;
	}

	aingle_value_get_by_name(&value, "ab", &field, NULL);
	aingle_value_set_int(&field, 1);

	fprintf(stderr, "NUM_RECORDS1 = %d NUM_RECORDS2 = %d\n", NUM_RECORDS1, NUM_RECORDS2);

	for (i = 0; i < NUM_RECORDS1; i++) {
		aingle_file_writer_append_value(writer, &value);
	}

	aingle_file_writer_close(writer);

	/* Make sure the sync ends at a BUFFER_LEN boundary */
	if (fsize(filename) != BUFFER_LEN) {
		fprintf(stderr, "internal error\n");
		return EXIT_FAILURE;
	}

	aingle_file_writer_open(filename, &writer);

	for (i = 0; i < NUM_RECORDS2; i++) {
		aingle_file_writer_append_value(writer, &value);
	}

	aingle_file_writer_close(writer);

	/* Make sure the whole file ends at a BUFFER_LEN boundary. */
	if (fsize(filename) != 2 * BUFFER_LEN) {
	      fprintf(stderr, "internal error\n");
	      return EXIT_FAILURE;
	}

	aingle_value_iface_decref(iface);
	aingle_value_decref(&value);
	aingle_schema_decref(schema);

	return EXIT_SUCCESS;
}


int main()
{
	int read_data_result;

	if (write_data()) {
		return EXIT_FAILURE;
	}

	read_data_result = read_data();
	remove(filename);

	return read_data_result;
}
