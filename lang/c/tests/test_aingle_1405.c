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

#define NUM_RECORDS 10

const char  PERSON_SCHEMA[] =
	"{"
	"    \"type\":\"record\","
	"    \"name\":\"Person\","
	"    \"fields\": ["
	"        {\"name\": \"ID\", \"type\": \"long\"},"
	"        {\"name\": \"First\", \"type\": \"string\"},"
	"        {\"name\": \"Last\", \"type\": \"string\"},"
	"        {\"name\": \"Phone\", \"type\": \"string\"},"
	"        {\"name\": \"Age\", \"type\": \"int\"}"
	"    ]"
	"}";

const char *file = "aingle_file.dat";

void print_aingle_value(aingle_value_t *value) {
	char *json;
	if (!aingle_value_to_json(value, 1, &json)) {
		printf("%s\n", json);
		free(json);
	}
}

int read_data() {
	int rval;
	int records_read = 0;

	aingle_file_reader_t reader;
	aingle_value_iface_t *iface;
	aingle_value_t value;

	aingle_file_reader(file, &reader);
	aingle_schema_t schema = aingle_file_reader_get_writer_schema(reader);

	iface = aingle_generic_class_from_schema(schema);
	aingle_generic_value_new(iface, &value);

	printf("\nReading...\n");
	while ((rval = aingle_file_reader_read_value(reader, &value)) == 0) {
		char  *json;

		if (aingle_value_to_json(&value, 1, &json)) {
			printf("Error converting value to JSON: %s\n",aingle_strerror());
		} else {
			printf("%s\n", json);
			free(json);
			records_read++;
		}

		aingle_value_reset(&value);
	}

	aingle_value_decref(&value);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(schema);
	aingle_file_reader_close(reader);

	if (rval != EOF || records_read != NUM_RECORDS) {
		fprintf(stderr, "Error: %s\n", aingle_strerror());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int write_data() {
	int  i;
	aingle_schema_t schema;
	aingle_schema_error_t error;
	aingle_file_writer_t writer;
	aingle_value_iface_t *iface;
	aingle_value_t value;

	if (aingle_schema_from_json(PERSON_SCHEMA, 0, &schema, &error)) {
		printf ("Unable to parse schema\n");
		return EXIT_FAILURE;
	}

	iface = aingle_generic_class_from_schema(schema);
	aingle_generic_value_new(iface, &value);

	if (aingle_file_writer_create(file, schema, &writer)) {
		printf ("There was an error creating file: %s\n", aingle_strerror());
		return EXIT_FAILURE;
	}

	printf("\nWriting...\n");
	for (i = 0; i < NUM_RECORDS; i++) {
		aingle_value_t  field;
		aingle_value_get_by_name(&value, "ID", &field, NULL);
		aingle_value_set_long(&field, (int64_t) i);

		aingle_value_get_by_name(&value, "Age", &field, NULL);
		aingle_value_set_int(&field, i);

		aingle_value_get_by_name(&value, "First", &field, NULL);
		aingle_value_set_string(&field, "Firstname");

		aingle_value_get_by_name(&value, "Last", &field, NULL);
		aingle_value_set_string(&field, "Lastname");


		aingle_value_get_by_name(&value, "Phone", &field, NULL);
		aingle_value_set_string(&field, "1234567");

		print_aingle_value(&value);

		aingle_file_writer_append_value(writer, &value);

		// Writing multiple blocks
		aingle_file_writer_close(writer);
		aingle_file_writer_open(file, &writer);

		aingle_value_reset(&value);
	}

	aingle_file_writer_close(writer);
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
	remove(file);

	return read_data_result;
}
