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

#include "aingle.h"
#include "aingle_private.h"
#include <stdio.h>
#include <stdlib.h>

static const char  *schema_json =
	"{"
	"  \"type\": \"record\","
	"  \"name\": \"test\","
	"  \"fields\": ["
	"    { \"name\": \"i\", \"type\": \"int\" },"
	"    { \"name\": \"l\", \"type\": \"long\" },"
	"    { \"name\": \"s\", \"type\": \"string\" },"
	"    {"
	"      \"name\": \"subrec\","
	"      \"type\": {"
	"        \"type\": \"record\","
	"        \"name\": \"sub\","
	"        \"fields\": ["
	"          { \"name\": \"f\", \"type\": \"float\" },"
	"          { \"name\": \"d\", \"type\": \"double\" }"
	"        ]"
	"      }"
	"    }"
	"  ]"
	"}";

static void
populate_complex_record(aingle_value_t *p_val)
{
	aingle_value_t  field;

	aingle_value_get_by_index(p_val, 0, &field, NULL);
	aingle_value_set_int(&field, 42);

	aingle_value_get_by_index(p_val, 1, &field, NULL);
	aingle_value_set_long(&field, 4242);

	aingle_wrapped_buffer_t  wbuf;
	aingle_wrapped_buffer_new_string(&wbuf, "Follow your bliss.");
	aingle_value_get_by_index(p_val, 2, &field, NULL);
	aingle_value_give_string_len(&field, &wbuf);

	aingle_value_t  subrec;
	aingle_value_get_by_index(p_val, 3, &subrec, NULL);

	aingle_value_get_by_index(&subrec, 0, &field, NULL);
	aingle_value_set_float(&field, 3.14159265);

	aingle_value_get_by_index(&subrec, 1, &field, NULL);
	aingle_value_set_double(&field, 2.71828183);
}

int main(void)
{
	int rval = 0;
	size_t len;
	static char  buf[4096];
	aingle_writer_t  writer;
	aingle_file_writer_t file_writer;
	aingle_file_reader_t file_reader;
	const char *outpath = "test-1379.aingle";

	aingle_schema_t  schema = NULL;
	aingle_schema_error_t  error = NULL;
	check(rval, aingle_schema_from_json(schema_json, strlen(schema_json), &schema, &error));

	aingle_value_iface_t  *iface = aingle_generic_class_from_schema(schema);

	aingle_value_t  val;
	aingle_generic_value_new(iface, &val);

	aingle_value_t  out;
	aingle_generic_value_new(iface, &out);

	/* create the val */
	aingle_value_reset(&val);
	populate_complex_record(&val);

	/* create the writers */
	writer = aingle_writer_memory(buf, sizeof(buf));
	check(rval, aingle_file_writer_create(outpath, schema, &file_writer));

	fprintf(stderr, "Writing to buffer\n");
	check(rval, aingle_value_write(writer, &val));

	fprintf(stderr, "Writing buffer to %s "
		"using aingle_file_writer_append_encoded()\n", outpath);
	len = aingle_writer_tell(writer);
	check(rval, aingle_file_writer_append_encoded(file_writer, buf, len));
	check(rval, aingle_file_writer_close(file_writer));

	check(rval, aingle_file_reader(outpath, &file_reader));
	fprintf(stderr, "Re-reading value to verify\n");
	check(rval, aingle_file_reader_read_value(file_reader, &out));
	fprintf(stderr, "Verifying value...");
	if (!aingle_value_equal(&val, &out)) {
		fprintf(stderr, "fail!\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "ok\n");
	check(rval, aingle_file_reader_close(file_reader));
	remove(outpath);

	aingle_writer_free(writer);
	aingle_value_decref(&out);
	aingle_value_decref(&val);
	aingle_value_iface_decref(iface);
	aingle_schema_decref(schema);

	exit(EXIT_SUCCESS);
}
