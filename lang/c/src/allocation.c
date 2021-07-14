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

#include <stdlib.h>
#include <string.h>

#include "aingle_private.h"
#include "aingle/allocation.h"
#include "aingle/data.h"
#include "aingle/legacy.h"

static void *
aingle_default_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
	AINGLE_UNUSED(ud);
	AINGLE_UNUSED(osize);

	if (nsize == 0) {
		free(ptr);
		return NULL;
	} else {
		return realloc(ptr, nsize);
	}
}

struct aingle_allocator_state  AINGLE_CURRENT_ALLOCATOR = {
	aingle_default_allocator,
	NULL
};

void aingle_set_allocator(aingle_allocator_t alloc, void *user_data)
{
	AINGLE_CURRENT_ALLOCATOR.alloc = alloc;
	AINGLE_CURRENT_ALLOCATOR.user_data = user_data;
}

void *aingle_calloc(size_t count, size_t size)
{
	void  *ptr = aingle_malloc(count * size);
	if (ptr != NULL) {
		memset(ptr, 0, count * size);
	}
	return ptr;
}

char *aingle_str_alloc(size_t str_size)
{
	size_t  buf_size = str_size + sizeof(size_t);

	void  *buf = aingle_malloc(buf_size);
	if (buf == NULL) {
		return NULL;
	}

	size_t  *size = (size_t *) buf;
	char  *new_str = (char *) (size + 1);

	*size = buf_size;

	return new_str;
}

char *aingle_strdup(const char *str)
{
	if (str == NULL) {
		return NULL;
	}

	size_t  str_size = strlen(str)+1;
	char *new_str = aingle_str_alloc(str_size);
	memcpy(new_str, str, str_size);

	//fprintf(stderr, "--- new  %" PRIsz " %p %s\n", *size, new_str, new_str);
	return new_str;
}

char *aingle_strndup(const char *str, size_t size)
{
	if (str == NULL) {
		return NULL;
	}

	char *new_str = aingle_str_alloc(size + 1);
	memcpy(new_str, str, size);
	new_str[size] = '\0';

	return new_str;
}

void aingle_str_free(char *str)
{
	size_t  *size = ((size_t *) str) - 1;
	//fprintf(stderr, "--- free %" PRIsz " %p %s\n", *size, str, str);
	aingle_free(size, *size);
}


void
aingle_alloc_free_func(void *ptr, size_t sz)
{
	aingle_free(ptr, sz);
}
