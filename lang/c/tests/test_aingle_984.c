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


/* Test code for JIRA Issue AINGLE-984. 
 * 
 * AINGLE-984: AIngle-C schema resolution fails on nested array
 * 
 * This program tests schema resolution for nested arrays. For the
 * purposes of this test, there are two schemas "old" and "new" which
 * are created by reading the same JSON schema.
 * 
 * The test creates and populates a nested array, and serializes it to
 * memory. The raw memory is written to a file, primarily to decouple
 * writing and reading. Note that the schema is not written to the
 * file. The nested array is also printed to the screen. 
 * 
 * The binary file is then read using two separate readers -- the
 * matched reader and the resolved reader.
 * 
 * In the matched reader case, the "old" and "new" schemas are known
 * to match, and therefore no schema resolution is done. The binary
 * buffer is deserialized into an aingle value and the nested array
 * encoded in the aingle value is printed to the screen. 
 * 
 * In the resolved reader case, the "old" and "new" schemas are not
 * known to match, and therefore schema resolution is performed. (Note
 * that the schemas *do* match, but we perform schema resolution
 * anyway, to test the resolution process). The schema resolution
 * appears to succeed. However, once the code tries to perform an
 * "aingle_value_read()" the code fails to read the nested array into
 * the aingle value.
 * 
 * Additionally valgrind indicates that conditional jumps are being
 * performed based on uninitialized values. 
 * 
 * AINGLE-C was compiled with CMAKE_INSTALL_PREFIX=ainglelib
 * The static library (libaingle.a) was copied into a subdirectory of ainglelib/lib/static
 * 
 * This file was compiled under Linux using:
 *   gcc -g aingle-984-test.c -o aingle984 -I../../build/ainglelib/include -L../../build/ainglelib/lib/static -laingle
 * 
 * The code was tested with valgrind using the command:
 *   valgrind -v --leak-check=full --track-origins=yes ./aingle984
 * 
 */


// Encode the following json string in NESTED_ARRAY
// {"type":"array", "items": {"type": "array", "items": "long"}}
// 
#define NESTED_ARRAY \
  "{\"type\":\"array\", \"items\": {\"type\": \"array\", \"items\": \"long\"}}"

aingle_schema_t schema_old = NULL;
aingle_schema_t schema_new = NULL;

/* Parse schema into a schema data structure */
void init_schema(void)
{
  aingle_schema_error_t error;
  if (aingle_schema_from_json(NESTED_ARRAY, sizeof(NESTED_ARRAY),
                            &schema_old, &error)) {
    printf( "Unable to parse old schema\n");
    exit(EXIT_FAILURE);
  }

  if (aingle_schema_from_json(NESTED_ARRAY, sizeof(NESTED_ARRAY),
                            &schema_new, &error)) {
    printf( "Unable to parse new schema\n");
    exit(EXIT_FAILURE);
  }
}

#define try(call, msg) \
	do { \
		if (call) { \
			printf( msg ":\n  %s\n", aingle_strerror()); \
			exit (EXIT_FAILURE);                       \
		} \
	} while (0)


/* The input aingle_value_t p_array should contain a nested array.
 * Print the fields of this nested array to the screen.
 */
int print_array_fields ( aingle_value_t *p_array )
{
  size_t idx;
  size_t length;
  aingle_type_t val_type;

  val_type = aingle_value_get_type( p_array );
  printf( "Main array type = %d\n", val_type );

  try( aingle_value_get_size( p_array, &length ),
       "Couldn't get array size" );
  printf( "Main array length = %d\n", (int) length );
  
  for ( idx = 0; idx < length; idx ++ )
  {
    aingle_value_t subarray;
    size_t sublength;
    size_t jdx;
    const char *unused;
    
    try ( aingle_value_get_by_index( p_array, idx, &subarray, &unused ),
          "Couldn't get subarray" );

    val_type = aingle_value_get_type( &subarray );
    printf( "Subarray type = %d\n", val_type );

    try( aingle_value_get_size( &subarray, &sublength ),
         "Couldn't get subarray size" );
    printf( "Subarray length = %d\n", (int) sublength );

    for ( jdx = 0; jdx < sublength; jdx++ )
    {
      aingle_value_t element;
      int64_t val;

      try ( aingle_value_get_by_index( &subarray, jdx, &element, &unused  ),
            "Couldn't get subarray element" );

      val_type = aingle_value_get_type( &element );

      try ( aingle_value_get_long( &element, &val ),
            "Couldn't get subarray element value" );

      printf( "nested_array[%d][%d]: type = %d value = %lld\n", 
              (int) idx, (int) jdx, (int) val_type, (long long) val );

    }
  }

  return 0;
}


/* The input aingle_value_t p_subarray should contain an array of long
 * integers. Add "elements" number of long integers to this array. Set
 * the values to be distinct based on the iteration parameter.
 */
int add_subarray( aingle_value_t *p_subarray,
                  int32_t elements, 
                  int32_t iteration )
{
  aingle_value_t element;
  size_t index;
  size_t idx;

  for ( idx = 0; idx < (size_t) elements; idx ++ )
  {
    // Append aingle array element to subarray
    try ( aingle_value_append( p_subarray, &element, &index ),
          "Error appending element in subarray" );

    try ( aingle_value_set_long( &element, (iteration+1)*100 + (iteration+1) ),
          "Error setting subarray element" );
  }

  return 0;
}


/* Create a nested array using the schema NESTED_ARRAY. Populate its
 * elements with unique values. Serialize the nested array to the
 * memory buffer in aingle_writer_t. The number of elements in the first
 * dimension of the nested array is "elements". The number of elements
 * in the second dimension of the nested array is hardcoded to 2.
 */
int add_array( aingle_writer_t writer, 
               int32_t elements )
{
  aingle_schema_t chosen_schema;
  aingle_value_iface_t *nested_array_class;
  aingle_value_t nested;
  int32_t idx;

  // Select (hardcode) schema to use
  chosen_schema = schema_old;

  // Create aingle class and value
  nested_array_class = aingle_generic_class_from_schema( chosen_schema );
  try ( aingle_generic_value_new( nested_array_class, &nested ), 
        "Error creating instance of record" );

  for ( idx = 0; idx < elements; idx ++ )
  {
    aingle_value_t subarray;
    size_t index;

    // Append aingle array element for top level array
    try ( aingle_value_append( &nested, &subarray, &index ),
          "Error appending subarray" );

    // Populate array element with subarray of length 2
#define SUBARRAY_LENGTH (2)
    try ( add_subarray( &subarray, SUBARRAY_LENGTH, idx ),
          "Error populating subarray" );
  }

  // Write the value to memory
  try ( aingle_value_write( writer, &nested ),
        "Unable to write nested into memory" );

  print_array_fields( &nested );

  // Release the record
  aingle_value_decref( &nested );
  aingle_value_iface_decref( nested_array_class );

  return 0;
}

/* Create a raw binary file containing a serialized version of a
 * nested array. This file will later be read by
 * read_nested_array_file().
 */
int write_nested_array_file ( int64_t buf_len, const char *raw_binary_file_name )
{
  char *buf;
  aingle_writer_t nested_writer;
  FILE *fid = NULL;

  fprintf( stdout, "Create %s\n", raw_binary_file_name );

  // Allocate a buffer
  buf = (char *) malloc( buf_len * sizeof( char ) );
  if ( buf == NULL )
  {
    printf( "There was an error creating the nested buffer %s.\n", raw_binary_file_name);
    exit(EXIT_FAILURE);
  }

  /* Create a new memory writer */
  nested_writer = aingle_writer_memory( buf, buf_len );
  if ( nested_writer == NULL )
  {
    printf( "There was an error creating the buffer for writing %s.\n", raw_binary_file_name);
    exit(EXIT_FAILURE);
  }

  /* Add an array containing 4 subarrays */
  printf( "before aingle_writer_tell %d\n", (int) aingle_writer_tell( nested_writer ) );
#define ARRAY_LENGTH (4)
  add_array( nested_writer, ARRAY_LENGTH );
  printf( "after aingle_writer_tell %d\n", (int) aingle_writer_tell( nested_writer ) );

  /* Serialize the nested array */
  printf( "Serialize the data to a file\n");

  /* Delete the nested array if it exists, and create a new one */
  remove(raw_binary_file_name);
  fid = fopen( raw_binary_file_name, "w+");
  if ( fid == NULL )
  {
    printf( "There was an error creating the file %s.\n", raw_binary_file_name);
    exit(EXIT_FAILURE);
  }
  fwrite( buf, 1, aingle_writer_tell( nested_writer ), fid );
  fclose(fid);
  aingle_writer_free( nested_writer );
  free(buf);
  return 0;
}


/* Read the raw binary file containing a serialized version of a
 * nested array, written by write_nested_array_file()
 */
int read_nested_array_file ( int64_t buf_len, 
                             const char *raw_binary_file_name, 
                             aingle_schema_t writer_schema,
                             aingle_schema_t reader_schema,
                             int use_resolving_reader
                           )
{

  char *buf;
  FILE *fid = NULL;
  aingle_reader_t nested_reader;
  int64_t file_len;

  // For Matched Reader and Resolving Reader
  aingle_value_iface_t *reader_class;
  aingle_value_t nested;
  
  // For Resolving Reader
  aingle_value_iface_t *resolver;
  aingle_value_t resolved_value;

  fprintf( stdout, "Use %s reader\n", use_resolving_reader ? "Resolving":"Matched" );

  // Allocate a buffer
  buf = (char *) calloc( buf_len, sizeof( char ) );
  if ( buf == NULL )
  {
    printf( "There was an error creating the buffer for reading %s.\n", raw_binary_file_name);
    exit(EXIT_FAILURE);
  }
  // Start with a garbage buffer
  memset(buf, 0xff, buf_len );

  // Read the file into the buffer
  fid = fopen( raw_binary_file_name, "r" );
  if ( fid == NULL )
  {
    printf( "There was an error reading the file %s.\n", raw_binary_file_name);
    exit(EXIT_FAILURE);
  }
  file_len = fread( buf, 1, buf_len, fid );
  printf( "Read %d bytes\n", (int) file_len );
  fclose(fid);

  if ( use_resolving_reader )
  {
    // Resolving Reader

    /* First resolve the writer and reader schemas */
    resolver = aingle_resolved_writer_new( writer_schema, reader_schema );
    if ( !resolver )
    {
      printf( "Could not create resolver\n");
      free(buf);
      exit(EXIT_FAILURE);
    }

    /* Create a value that the resolver can write into. This is just
     * an interface value, that is not directly read from.
     */
    if ( aingle_resolved_writer_new_value( resolver, &resolved_value ) )
    {
      aingle_value_iface_decref( resolver );
      free(buf);      
      exit(EXIT_FAILURE);
    }

    /* Then create the value with the reader schema, that we are going
     * to use to read from.
     */
    reader_class = aingle_generic_class_from_schema(reader_schema);
    try ( aingle_generic_value_new( reader_class, &nested ),
          "Error creating instance of nested array" );

    // When we read the memory using the resolved writer, we want to
    // populate the instance of the value with the reader schema. This
    // is done by set_dest.
    aingle_resolved_writer_set_dest(&resolved_value, &nested);

    // Create a memory reader
    nested_reader = aingle_reader_memory( buf, buf_len );

    if ( aingle_value_read( nested_reader, &resolved_value ) )
    {
      printf( "AIngle value read failed\n" );

      aingle_value_decref( &nested );
      aingle_value_iface_decref( reader_class );
      aingle_value_iface_decref( resolver );
      aingle_value_decref( &resolved_value );

      exit(EXIT_FAILURE);
    }
  }
  else
  {
    // Matched Reader
    reader_class = aingle_generic_class_from_schema(reader_schema);

    try ( aingle_generic_value_new( reader_class, &nested ),
          "Error creating instance of nested array" );

    // Send the memory in the buffer into the reader
    nested_reader = aingle_reader_memory( buf, buf_len );

    try ( aingle_value_read( nested_reader, &nested ),
          "Could not read value from memory" );
  }


  /* Now the resolved record has been read into "nested" which is
   * a value of type reader_class
   */
  print_array_fields( &nested );

  if ( use_resolving_reader )
  {
    // Resolving Reader
    aingle_value_decref( &nested );
    aingle_value_iface_decref( reader_class );
    aingle_value_iface_decref( resolver );
    aingle_value_decref( &resolved_value );
  }
  else
  {
    // Matched Reader
    aingle_value_decref( &nested );    
    aingle_value_iface_decref( reader_class );
  }

  fprintf( stdout, "Done.\n\n");
  aingle_reader_free( nested_reader );
  free(buf);
  return 0;
}


/* Top level function to impelement a test for the JIRA issue
 * AINGLE-984. See detailed documentation at the top of this file.
 */
int main(void)
{
  const char *raw_binary_file_name = "nested_array.bin";
  int64_t buf_len = 2048;
  int use_resolving_reader;

  /* Initialize the schema structure from JSON */
  init_schema();

  printf( "Write the serialized nested array to %s\n", raw_binary_file_name );

  write_nested_array_file( buf_len, raw_binary_file_name );

  printf("\nNow read all the array back out\n\n");

  for ( use_resolving_reader = 0; use_resolving_reader < 2; use_resolving_reader++ )
  {
    read_nested_array_file( buf_len, 
                            raw_binary_file_name,
                            schema_old,
                            schema_new,
                            use_resolving_reader
                          );
  }

  // Close out schemas
  aingle_schema_decref(schema_old);
  aingle_schema_decref(schema_new);

  // Remove the binary file
  remove(raw_binary_file_name);
  
  printf("\n");
  return 0;
}
