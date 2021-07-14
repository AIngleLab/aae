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

#ifndef AINGLE_ALLOCATION_H
#define AINGLE_ALLOCATION_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <stdlib.h>

/*
 * Allocation interface.  You can provide a custom allocator for the
 * library, should you wish.  The allocator is provided as a single
 * generic function, which can emulate the standard malloc, realloc, and
 * free functions.  The design of this allocation interface is inspired
 * by the implementation of the Lua interpreter.
 *
 * The ptr parameter will be the location of any existing memory
 * buffer.  The osize parameter will be the size of this existing
 * buffer.  If ptr is NULL, then osize will be 0.  The nsize parameter
 * will be the size of the new buffer, or 0 if the new buffer should be
 * freed.
 *
 * If nsize is 0, then the allocation function must return NULL.  If
 * nsize is not 0, then it should return NULL if the allocation fails.
 */

typedef void *
(*aingle_allocator_t)(void *user_data, void *ptr, size_t osize, size_t nsize);

void aingle_set_allocator(aingle_allocator_t alloc, void *user_data);

struct aingle_allocator_state {
	aingle_allocator_t  alloc;
	void  *user_data;
};

extern struct aingle_allocator_state  AINGLE_CURRENT_ALLOCATOR;

#define aingle_realloc(ptr, osz, nsz)          \
	(AINGLE_CURRENT_ALLOCATOR.alloc        \
	 (AINGLE_CURRENT_ALLOCATOR.user_data,  \
	  (ptr), (osz), (nsz)))

#define aingle_malloc(sz) (aingle_realloc(NULL, 0, (sz)))
#define aingle_free(ptr, osz) (aingle_realloc((ptr), (osz), 0))

#define aingle_new(type) (aingle_realloc(NULL, 0, sizeof(type)))
#define aingle_freet(type, ptr) (aingle_realloc((ptr), sizeof(type), 0))

void *aingle_calloc(size_t count, size_t size);

/*
 * This is probably too clever for our own good, but when we duplicate a
 * string, we actually store its size in the same allocated memory
 * buffer.  That lets us free the string later, without having to call
 * strlen to get its size, and without the containing struct having to
 * manually store the strings length.
 *
 * This means that any string return by aingle_strdup MUST be freed using
 * aingle_str_free, and the only thing that can be passed into
 * aingle_str_free is a string created via aingle_strdup.
 */

char *aingle_str_alloc(size_t str_size);
char *aingle_strdup(const char *str);
char *aingle_strndup(const char *str, size_t size);
void aingle_str_free(char *str);

CLOSE_EXTERN
#endif
