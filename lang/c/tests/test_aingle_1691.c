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

/**
 * AINGLE-1691 test case - support of string-field JSON schemas.
 */
#include "aingle.h"
#include "aingle_private.h"
#include <stdio.h>
#include <stdlib.h>

static const char *json_schemas[] = {
        /* These two schemas are functionally equivalent. */
        "{ \"type\": \"string\" }", /* Object wrapped */
        "\"string\"",               /* JSON string field */
        NULL
};




int main(void)
{
        int pass;

        for (pass = 0 ; json_schemas[pass] ; pass++) {
                int rval = 0;
                size_t len;
                static char  buf[4096];
                aingle_writer_t  writer;
                aingle_file_writer_t file_writer;
                aingle_file_reader_t file_reader;
                aingle_schema_t  schema = NULL;
                aingle_schema_error_t  error = NULL;
                char outpath[64];
                const char *json_schema = json_schemas[pass];

                printf("pass %d with schema %s\n", pass, json_schema);
                check(rval, aingle_schema_from_json(json_schema, strlen(json_schema),
                                                  &schema, &error));

                aingle_value_iface_t  *iface = aingle_generic_class_from_schema(schema);

                aingle_value_t  val;
                aingle_generic_value_new(iface, &val);

                aingle_value_t  out;
                aingle_generic_value_new(iface, &out);

                /* create the val */
                aingle_value_reset(&val);
                aingle_value_set_string(&val, "test-1691");

                /* Write value to file */
                snprintf(outpath, sizeof(outpath), "test-1691-%d.aingle", pass);

                /* create the writers */
                writer = aingle_writer_memory(buf, sizeof(buf));
                check(rval, aingle_file_writer_create(outpath, schema, &file_writer));

                check(rval, aingle_value_write(writer, &val));

                len = aingle_writer_tell(writer);
                check(rval, aingle_file_writer_append_encoded(file_writer, buf, len));
                check(rval, aingle_file_writer_close(file_writer));

                /* Read the value back */
                check(rval, aingle_file_reader(outpath, &file_reader));
                check(rval, aingle_file_reader_read_value(file_reader, &out));
                if (!aingle_value_equal(&val, &out)) {
                        fprintf(stderr, "fail!\n");
                        exit(EXIT_FAILURE);
                }
                fprintf(stderr, "pass %d: ok: schema %s\n", pass, json_schema);
                check(rval, aingle_file_reader_close(file_reader));
                remove(outpath);
                
                aingle_writer_free(writer);
                aingle_value_decref(&out);
                aingle_value_decref(&val);
                aingle_value_iface_decref(iface);
                aingle_schema_decref(schema);
        }

	exit(EXIT_SUCCESS);
}
