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
#ifndef AINGLE_ENCODING_H
#define AINGLE_ENCODING_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/platform.h>
#include "aingle/io.h"

/*
 * TODO: this will need more functions when JSON encoding is added 
 */
struct aingle_encoding_t {
	const char *description;
	/*
	 * string 
	 */
	int (*read_string) (aingle_reader_t reader, char **s, int64_t *len);
	int (*skip_string) (aingle_reader_t reader);
	int (*write_string) (aingle_writer_t writer, const char *s);
	 int64_t(*size_string) (aingle_writer_t writer, const char *s);
	/*
	 * bytes 
	 */
	int (*read_bytes) (aingle_reader_t reader, char **bytes, int64_t * len);
	int (*skip_bytes) (aingle_reader_t reader);
	int (*write_bytes) (aingle_writer_t writer,
			    const char *bytes, const int64_t len);
	 int64_t(*size_bytes) (aingle_writer_t writer,
			       const char *bytes, const int64_t len);
	/*
	 * int 
	 */
	int (*read_int) (aingle_reader_t reader, int32_t * i);
	int (*skip_int) (aingle_reader_t reader);
	int (*write_int) (aingle_writer_t writer, const int32_t i);
	 int64_t(*size_int) (aingle_writer_t writer, const int32_t i);
	/*
	 * long 
	 */
	int (*read_long) (aingle_reader_t reader, int64_t * l);
	int (*skip_long) (aingle_reader_t reader);
	int (*write_long) (aingle_writer_t writer, const int64_t l);
	 int64_t(*size_long) (aingle_writer_t writer, const int64_t l);
	/*
	 * float 
	 */
	int (*read_float) (aingle_reader_t reader, float *f);
	int (*skip_float) (aingle_reader_t reader);
	int (*write_float) (aingle_writer_t writer, const float f);
	 int64_t(*size_float) (aingle_writer_t writer, const float f);
	/*
	 * double 
	 */
	int (*read_double) (aingle_reader_t reader, double *d);
	int (*skip_double) (aingle_reader_t reader);
	int (*write_double) (aingle_writer_t writer, const double d);
	 int64_t(*size_double) (aingle_writer_t writer, const double d);
	/*
	 * boolean 
	 */
	int (*read_boolean) (aingle_reader_t reader, int8_t * b);
	int (*skip_boolean) (aingle_reader_t reader);
	int (*write_boolean) (aingle_writer_t writer, const int8_t b);
	 int64_t(*size_boolean) (aingle_writer_t writer, const int8_t b);
	/*
	 * null 
	 */
	int (*read_null) (aingle_reader_t reader);
	int (*skip_null) (aingle_reader_t reader);
	int (*write_null) (aingle_writer_t writer);
	 int64_t(*size_null) (aingle_writer_t writer);
};
typedef struct aingle_encoding_t aingle_encoding_t;

#define AINGLE_WRITE(writer, buf, len) \
{ int rval = aingle_write( writer, buf, len ); if(rval) return rval; }
#define AINGLE_READ(reader, buf, len)  \
{ int rval = aingle_read( reader, buf, len ); if(rval) return rval; }
#define AINGLE_SKIP(reader, len) \
{ int rval = aingle_skip( reader, len); if (rval) return rval; }

extern const aingle_encoding_t aingle_binary_encoding;	/* in
							 * encoding_binary 
							 */
CLOSE_EXTERN
#endif
