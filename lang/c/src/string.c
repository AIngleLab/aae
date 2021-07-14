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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "aingle_private.h"
#include "aingle/data.h"
#include "aingle/allocation.h"
#include "aingle/errors.h"

#ifndef AINGLE_STRING_DEBUG
#define AINGLE_STRING_DEBUG 0
#endif

#if AINGLE_STRING_DEBUG
#include <stdio.h>
#define DEBUG(...) \
	do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
	} while (0)
#else
#define DEBUG(...)  /* don't print messages */
#endif


/*
 * A resizable wrapped buffer implementation.  This implementation makes
 * actual copies in its copy method; if we wanted a zero-copy solution
 * here, then we'd have to keep track of all copies of the buffer, so
 * that we can update pointers whenever the buffer is resized (since
 * this might change the location of the memory region).
 */

struct aingle_wrapped_resizable {
	size_t  buf_size;
};

#define aingle_wrapped_resizable_size(sz) \
	(sizeof(struct aingle_wrapped_resizable) + (sz))

static void
aingle_wrapped_resizable_free(aingle_wrapped_buffer_t *self)
{
	DEBUG("--- Freeing resizable <%p:%" PRIsz "> (%p)", self->buf, self->size, self->user_data);
	struct aingle_wrapped_resizable  *resizable = (struct aingle_wrapped_resizable *) self->user_data;
	aingle_free(resizable, aingle_wrapped_resizable_size(resizable->buf_size));
}

static int
aingle_wrapped_resizable_resize(aingle_wrapped_buffer_t *self, size_t desired)
{
	struct aingle_wrapped_resizable  *resizable = (struct aingle_wrapped_resizable *) self->user_data;

	/*
	 * If we've already allocated enough memory for the desired
	 * size, there's nothing to do.
	 */

	if (resizable->buf_size >= desired) {
		return 0;
	}

	size_t  new_buf_size = resizable->buf_size * 2;
	if (desired > new_buf_size) {
		new_buf_size = desired;
	}

	DEBUG("--- Resizing <%p:%" PRIsz "> (%p) -> %" PRIsz,
	      self->buf, self->buf_size, self->user_data, new_buf_size);

	struct aingle_wrapped_resizable  *new_resizable =
	    (struct aingle_wrapped_resizable *) aingle_realloc(resizable,
		         aingle_wrapped_resizable_size(resizable->buf_size),
			 aingle_wrapped_resizable_size(new_buf_size));
	if (new_resizable == NULL) {
		return ENOMEM;
	}
	DEBUG("--- New buffer <%p:%" PRIsz ">", new_buf, new_buf_size);

	new_resizable->buf_size = new_buf_size;

	char  *old_buf = (char *) resizable;
	char  *new_buf = (char *) new_resizable;

	ptrdiff_t  offset = (char *) self->buf - old_buf;
	DEBUG("--- Old data pointer is %p", self->buf);
	self->buf = new_buf + offset;
	self->user_data = new_resizable;
	DEBUG("--- New data pointer is %p", self->buf);
	return 0;
}

static int
aingle_wrapped_resizable_new(aingle_wrapped_buffer_t *dest, size_t buf_size)
{
	size_t  allocated_size = aingle_wrapped_resizable_size(buf_size);
	struct aingle_wrapped_resizable  *resizable =
	    (struct aingle_wrapped_resizable *) aingle_malloc(allocated_size);
	if (resizable == NULL) {
		return ENOMEM;
	}

	resizable->buf_size = buf_size;

	dest->buf = ((char *) resizable) + sizeof(struct aingle_wrapped_resizable);
	DEBUG("--- Creating resizable <%p:%" PRIsz "> (%p)", dest->buf, buf_size, resizable);
	dest->size = buf_size;
	dest->user_data = resizable;
	dest->free = aingle_wrapped_resizable_free;
	dest->copy = NULL;
	dest->slice = NULL;
	return 0;
}

#define is_resizable(buf) \
	((buf).free == aingle_wrapped_resizable_free)



void
aingle_raw_string_init(aingle_raw_string_t *str)
{
	memset(str, 0, sizeof(aingle_raw_string_t));
}


void
aingle_raw_string_clear(aingle_raw_string_t *str)
{
	/*
	 * If the string's buffer is one that we control, then we don't
	 * free it; that lets us reuse the storage on the next call to
	 * aingle_raw_string_set[_length].
	 */

	if (is_resizable(str->wrapped)) {
		DEBUG("--- Clearing resizable buffer");
		str->wrapped.size = 0;
	} else {
		DEBUG("--- Freeing wrapped buffer");
		aingle_wrapped_buffer_free(&str->wrapped);
		aingle_raw_string_init(str);
	}
}


void
aingle_raw_string_done(aingle_raw_string_t *str)
{
	aingle_wrapped_buffer_free(&str->wrapped);
	aingle_raw_string_init(str);
}


/**
 * Makes sure that the string's buffer is one that we allocated
 * ourselves, and that the buffer is big enough to hold a string of the
 * given length.
 */

static int
aingle_raw_string_ensure_buf(aingle_raw_string_t *str, size_t length)
{
	int  rval;

	DEBUG("--- Ensuring resizable buffer of size %" PRIsz, length);
	if (is_resizable(str->wrapped)) {
		/*
		 * If we've already got a resizable buffer, just have it
		 * resize itself.
		 */

		return aingle_wrapped_resizable_resize(&str->wrapped, length);
	} else {
		/*
		 * Stash a copy of the old wrapped buffer, and then
		 * create a new resizable buffer to store our content
		 * in.
		 */

		aingle_wrapped_buffer_t  orig = str->wrapped;
		check(rval, aingle_wrapped_resizable_new(&str->wrapped, length));

		/*
		 * If there was any content in the old wrapped buffer,
		 * copy it into the new resizable one.
		 */

		if (orig.size > 0) {
			size_t  to_copy =
			    (orig.size < length)? orig.size: length;
			memcpy((void *) str->wrapped.buf, orig.buf, to_copy);
		}
		aingle_wrapped_buffer_free(&orig);

		return 0;
	}
}


void
aingle_raw_string_set_length(aingle_raw_string_t *str,
			   const void *src, size_t length)
{
	aingle_raw_string_ensure_buf(str, length+1);
	memcpy((void *) str->wrapped.buf, src, length);
	((char *) str->wrapped.buf)[length] = '\0';
	str->wrapped.size = length;
}


void aingle_raw_string_append_length(aingle_raw_string_t *str,
				   const void *src,
				   size_t length)
{
	if (aingle_raw_string_length(str) == 0) {
		return aingle_raw_string_set_length(str, src, length);
	}

	aingle_raw_string_ensure_buf(str, str->wrapped.size + length);
	memcpy((char *) str->wrapped.buf + str->wrapped.size, src, length);
	str->wrapped.size += length;
}


void
aingle_raw_string_set(aingle_raw_string_t *str, const char *src)
{
	size_t  length = strlen(src);
	aingle_raw_string_ensure_buf(str, length+1);
	memcpy((void *) str->wrapped.buf, src, length+1);
	str->wrapped.size = length+1;
}


void
aingle_raw_string_append(aingle_raw_string_t *str, const char *src)
{
	if (aingle_raw_string_length(str) == 0) {
		return aingle_raw_string_set(str, src);
	}

	/* Assume that str->wrapped.size includes a NUL terminator */
	size_t  length = strlen(src);
	aingle_raw_string_ensure_buf(str, str->wrapped.size + length);
	memcpy((char *) str->wrapped.buf + str->wrapped.size - 1, src, length+1);
	str->wrapped.size += length;
}


void
aingle_raw_string_give(aingle_raw_string_t *str,
		     aingle_wrapped_buffer_t *src)
{
	DEBUG("--- Giving control of <%p:%" PRIsz "> (%p) to string",
	      src->buf, src->size, src);
	aingle_wrapped_buffer_free(&str->wrapped);
	aingle_wrapped_buffer_move(&str->wrapped, src);
}

int
aingle_raw_string_grab(const aingle_raw_string_t *str,
		     aingle_wrapped_buffer_t *dest)
{
	return aingle_wrapped_buffer_copy(dest, &str->wrapped, 0, str->wrapped.size);
}


int
aingle_raw_string_equals(const aingle_raw_string_t *str1,
		       const aingle_raw_string_t *str2)
{
	if (str1 == str2) {
		return 1;
	}

	if (!str1 || !str2) {
		return 0;
	}

	if (str1->wrapped.size != str2->wrapped.size) {
		return 0;
	}

	return (memcmp(str1->wrapped.buf, str2->wrapped.buf,
		       str1->wrapped.size) == 0);
}
