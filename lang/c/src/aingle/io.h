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

#ifndef AINGLE_IO_H
#define AINGLE_IO_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/platform.h>
#include <stdio.h>

#include <aingle/basics.h>
#include <aingle/legacy.h>
#include <aingle/schema.h>
#include <aingle/value.h>

typedef struct aingle_reader_t_ *aingle_reader_t;
typedef struct aingle_writer_t_ *aingle_writer_t;

/*
 * io
 */

aingle_reader_t aingle_reader_file(FILE * fp);
aingle_reader_t aingle_reader_file_fp(FILE * fp, int should_close);
aingle_writer_t aingle_writer_file(FILE * fp);
aingle_writer_t aingle_writer_file_fp(FILE * fp, int should_close);
aingle_reader_t aingle_reader_memory(const char *buf, int64_t len);
aingle_writer_t aingle_writer_memory(const char *buf, int64_t len);

void
aingle_reader_memory_set_source(aingle_reader_t reader, const char *buf, int64_t len);

void
aingle_writer_memory_set_dest(aingle_writer_t writer, const char *buf, int64_t len);

int aingle_read(aingle_reader_t reader, void *buf, int64_t len);
int aingle_skip(aingle_reader_t reader, int64_t len);
int aingle_write(aingle_writer_t writer, void *buf, int64_t len);

void aingle_reader_reset(aingle_reader_t reader);

void aingle_writer_reset(aingle_writer_t writer);
int64_t aingle_writer_tell(aingle_writer_t writer);
void aingle_writer_flush(aingle_writer_t writer);

void aingle_writer_dump(aingle_writer_t writer, FILE * fp);
void aingle_reader_dump(aingle_reader_t reader, FILE * fp);

int aingle_reader_is_eof(aingle_reader_t reader);

void aingle_reader_free(aingle_reader_t reader);
void aingle_writer_free(aingle_writer_t writer);

int aingle_schema_to_json(const aingle_schema_t schema, aingle_writer_t out);

/*
 * Reads a binary-encoded AIngle value from the given reader object,
 * storing the result into dest.
 */

int
aingle_value_read(aingle_reader_t reader, aingle_value_t *dest);

/*
 * Writes a binary-encoded AIngle value to the given writer object.
 */

int
aingle_value_write(aingle_writer_t writer, aingle_value_t *src);

/*
 * Returns the size of the binary encoding of the given AIngle value.
 */

int
aingle_value_sizeof(aingle_value_t *src, size_t *size);


/* File object container */
typedef struct aingle_file_reader_t_ *aingle_file_reader_t;
typedef struct aingle_file_writer_t_ *aingle_file_writer_t;

int aingle_file_writer_create(const char *path, aingle_schema_t schema,
			    aingle_file_writer_t * writer);
int aingle_file_writer_create_fp(FILE *fp, const char *path, int should_close,
				aingle_schema_t schema, aingle_file_writer_t * writer);
int aingle_file_writer_create_with_codec(const char *path,
				aingle_schema_t schema, aingle_file_writer_t * writer,
				const char *codec, size_t block_size);
int aingle_file_writer_create_with_codec_fp(FILE *fp, const char *path, int should_close,
				aingle_schema_t schema, aingle_file_writer_t * writer,
				const char *codec, size_t block_size);
int aingle_file_writer_open(const char *path, aingle_file_writer_t * writer);
int aingle_file_writer_open_bs(const char *path, aingle_file_writer_t * writer, size_t block_size);
int aingle_file_reader(const char *path, aingle_file_reader_t * reader);
int aingle_file_reader_fp(FILE *fp, const char *path, int should_close,
			aingle_file_reader_t * reader);

aingle_schema_t
aingle_file_reader_get_writer_schema(aingle_file_reader_t reader);

int aingle_file_writer_sync(aingle_file_writer_t writer);
int aingle_file_writer_flush(aingle_file_writer_t writer);
int aingle_file_writer_close(aingle_file_writer_t writer);

int aingle_file_reader_close(aingle_file_reader_t reader);

int
aingle_file_reader_read_value(aingle_file_reader_t reader, aingle_value_t *dest);

int
aingle_file_writer_append_value(aingle_file_writer_t writer, aingle_value_t *src);

int
aingle_file_writer_append_encoded(aingle_file_writer_t writer,
				const void *buf, int64_t len);

/*
 * Legacy aingle_datum_t API
 */

int aingle_read_data(aingle_reader_t reader,
		   aingle_schema_t writer_schema,
		   aingle_schema_t reader_schema, aingle_datum_t * datum);
int aingle_skip_data(aingle_reader_t reader, aingle_schema_t writer_schema);
int aingle_write_data(aingle_writer_t writer,
		    aingle_schema_t writer_schema, aingle_datum_t datum);
int64_t aingle_size_data(aingle_writer_t writer,
		       aingle_schema_t writer_schema, aingle_datum_t datum);

int aingle_file_writer_append(aingle_file_writer_t writer, aingle_datum_t datum);

int aingle_file_reader_read(aingle_file_reader_t reader,
			  aingle_schema_t readers_schema, aingle_datum_t * datum);

CLOSE_EXTERN
#endif
