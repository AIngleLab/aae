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

#ifndef AINGLE_CODEC_H
#define	AINGLE_CODEC_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/platform.h>

enum aingle_codec_type_t {
	AINGLE_CODEC_NULL,
	AINGLE_CODEC_DEFLATE,
	AINGLE_CODEC_LZMA,
	AINGLE_CODEC_SNAPPY
};
typedef enum aingle_codec_type_t aingle_codec_type_t;

struct aingle_codec_t_ {
	const char * name;
	aingle_codec_type_t type;
	int64_t block_size;
	int64_t used_size;
	void * block_data;
	void * codec_data;
};
typedef struct aingle_codec_t_* aingle_codec_t;

int aingle_codec(aingle_codec_t c, const char *type);
int aingle_codec_reset(aingle_codec_t c);
int aingle_codec_encode(aingle_codec_t c, void * data, int64_t len);
int aingle_codec_decode(aingle_codec_t c, void * data, int64_t len);

CLOSE_EXTERN
#endif
