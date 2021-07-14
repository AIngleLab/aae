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

static const char *filename = "aingle_file.dat";

static const char PERSON_SCHEMA[] =
	"{"
	"    \"type\":\"record\","
	"    \"name\":\"Person\","
	"    \"fields\": ["
	"        {\"name\": \"ab\", \"type\": \"int\"}"
	"    ]"
	"}";

static int read_data() {
	int rval;
	int records_read = 0;

	aingle_file_reader_t reader;
	aingle_value_iface_t *iface;
	aingle_value_t value;

	fprintf(stderr, "\nReading...\n");

	rval = aingle_file_reader(filename, &reader);

	if (rval) {
		fprintf(stderr, "Error: %s\n", aingle_strerror());
		return -1;
	}

	aingle_schema_t schema = aingle_file_reader_get_writer_schema(reader);

	iface = aingle_generic_class_from_schema(schema);
	aingle_generic_value_new(iface, &value);

	while ((rval = aingle_file_reader_read_value(reader, &value)) == 0) {
		aingle_value_t field;
		int32_t val;
		aingle_value_get_by_index(&value, 0, &field, NULL);
		aingle_value_get_int(&field, &val);
		fprintf(stderr, "value = %d\n", val);
		records_read++;
		aingle_value_reset(&value);
	}

	aingle_value_decref(&value);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(schema);
	aingle_file_reader_close(reader);

	fprintf(stderr, "read %d records.\n", records_read);

	if (rval != EOF) {
		fprintf(stderr, "Error: %s\n", aingle_strerror());
		return -1;
	}

	return records_read;
}

static int read_data_datum() {
	int rval;
	int records_read = 0;

	aingle_file_reader_t reader;
	aingle_datum_t datum;

	fprintf(stderr, "\nReading...\n");

	rval = aingle_file_reader(filename, &reader);

	if (rval) {
		fprintf(stderr, "Error using 'datum': %s\n", aingle_strerror());
		return -1;
	}

	aingle_schema_t schema = aingle_file_reader_get_writer_schema(reader);

	while ((rval = aingle_file_reader_read(reader, schema, &datum)) == 0) {
		aingle_datum_t val_datum;
		int32_t val;
		if (aingle_record_get(datum, "ab", &val_datum)) {
			fprintf(stderr, "Error getting value: %s\n", aingle_strerror());
			return -1;
		}
		aingle_int32_get(val_datum, &val);
		fprintf(stderr, "value = %d\n", val);
		records_read++;
		aingle_datum_decref(datum);
	}

	aingle_schema_decref(schema);
	aingle_file_reader_close(reader);

	fprintf(stderr, "read %d records using 'datum'.\n", records_read);

	if (rval != EOF) {
		fprintf(stderr, "Error using 'datum': %s\n", aingle_strerror());
		return -1;
	}

	return records_read;
}

static int write_data(int n_records) {
	int  i;
	aingle_schema_t schema;
	aingle_schema_error_t error;
	aingle_file_writer_t writer;
	aingle_value_iface_t *iface;
	aingle_value_t value;

	fprintf(stderr, "\nWriting...\n");

	if (aingle_schema_from_json(PERSON_SCHEMA, 0, &schema, &error)) {
		fprintf(stderr, "Unable to parse schema\n");
		return -1;
	}

	if (aingle_file_writer_create(filename, schema, &writer)) {
		fprintf(stderr, "There was an error creating file: %s\n", aingle_strerror());
		return -1;
	}

	iface = aingle_generic_class_from_schema(schema);
	aingle_generic_value_new(iface, &value);

	aingle_value_t field;

	aingle_value_get_by_index(&value, 0, &field, NULL);
	aingle_value_set_int(&field, 123);

	for (i = 0; i < n_records; i++) {
		if (aingle_file_writer_append_value(writer, &value)) {
			fprintf(stderr, "There was an error writing file: %s\n", aingle_strerror());
			return -1;
		}
	}

	if (aingle_file_writer_close(writer)) {
		fprintf(stderr, "There was an error creating file: %s\n", aingle_strerror());
		return -1;
	}

	aingle_value_decref(&value);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(schema);

	return n_records;
}

static int test_n_records(int n_records) {
	int res = 0;

	if (write_data(n_records) != n_records) {
		remove(filename);
		return -1;
	}

	if (read_data() != n_records) {
		remove(filename);
		return -1;
	}

	if (read_data_datum() != n_records) {
		remove(filename);
		return -1;
	}

	remove(filename);
	return 0;
}

int main()
{
	if (test_n_records(1) < 0) {
		return EXIT_FAILURE;
	}

	if (test_n_records(0) < 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
