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

const char  PERSON_SCHEMA[] =
"{\"type\":\"record\",\
  \"name\":\"Person\",\
  \"fields\":[\
     {\"name\": \"ID\", \"type\": \"int\"}]}";

const char *dbname = "test.db";
aingle_schema_t schema;

void add_record (aingle_file_writer_t writer)
{
	aingle_datum_t main_datum = aingle_record(schema);
	aingle_datum_t id_datum = aingle_int32(1);

	if (aingle_record_set (main_datum, "ID", id_datum))
	{
		printf ("Unable to create datum");
		exit (EXIT_FAILURE);
	}

	aingle_file_writer_append (writer, main_datum);

	aingle_datum_decref (id_datum);
	aingle_datum_decref (main_datum);
}

void create_database()
{
	aingle_file_writer_t writer;

	if (aingle_schema_from_json_literal (PERSON_SCHEMA, &schema))
	{
		printf ("Unable to parse schema\n");
		exit (EXIT_FAILURE);
	}

	if (aingle_file_writer_create ("test.db", schema, &writer))
	{
		printf ("There was an error creating db: %s\n", aingle_strerror());
		exit (EXIT_FAILURE);
	}

	add_record (writer);

	aingle_file_writer_flush (writer);
	aingle_file_writer_close (writer);
}


int main()
{
	aingle_file_writer_t writer;

	create_database();

	aingle_file_writer_open (dbname, &writer);
	add_record (writer);

	aingle_file_writer_flush (writer);
	aingle_file_writer_close (writer);

    aingle_schema_decref(schema);

	remove (dbname);

	return EXIT_SUCCESS;
}
