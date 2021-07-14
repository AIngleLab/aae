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

#define SIMPLE_ARRAY \
"{\"type\": \"array\", \"items\": \"long\"}"


int main(void)
{
  aingle_schema_t schema = NULL;
  aingle_schema_error_t error;
  aingle_value_iface_t *simple_array_class;
  aingle_value_t simple;

  /* Initialize the schema structure from JSON */
  if (aingle_schema_from_json(SIMPLE_ARRAY, sizeof(SIMPLE_ARRAY),
                            &schema, &error)) {
    fprintf(stdout, "Unable to parse schema\n");
    exit(EXIT_FAILURE);
  }

  // Create aingle class and value
  simple_array_class = aingle_generic_class_from_schema( schema );
  if ( simple_array_class == NULL )
  {
    fprintf(stdout, "Unable to create simple array class\n");
    exit(EXIT_FAILURE);
  }

  if ( aingle_generic_value_new( simple_array_class, &simple ) )
  {
    fprintf(stdout, "Error creating instance of record\n" );
    exit(EXIT_FAILURE);
  }

  // Release the aingle class and value
  aingle_value_decref( &simple );
  aingle_value_iface_decref( simple_array_class );
  aingle_schema_decref(schema);
  
  return 0;

}
