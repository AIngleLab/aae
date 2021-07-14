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

#ifndef AINGLE_DATA_H
#define AINGLE_DATA_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <stdlib.h>
#include <string.h>

/*
 * This file defines some helper data structures that are used within
 * AIngle, and in the schema-specific types created by ainglecc.
 */


/*---------------------------------------------------------------------
 * Arrays
 */

/**
 * A resizeable array of fixed-size elements.
 */

typedef struct aingle_raw_array {
	size_t  element_size;
	size_t  element_count;
	size_t  allocated_size;
	void  *data;
} aingle_raw_array_t;

/**
 * Initializes a new aingle_raw_array_t that you've allocated yourself.
 */

void aingle_raw_array_init(aingle_raw_array_t *array, size_t element_size);

/**
 * Finalizes an aingle_raw_array_t.
 */

void aingle_raw_array_done(aingle_raw_array_t *array);

/**
 * Clears an aingle_raw_array_t.  This does not deallocate any space; this
 * allows us to reuse the underlying array buffer as we start to re-add
 * elements to the array.
 */

void aingle_raw_array_clear(aingle_raw_array_t *array);

/**
 * Ensures that there is enough allocated space to store the given
 * number of elements in an aingle_raw_array_t.  If we can't allocate that
 * much space, we return ENOMEM.
 */

int
aingle_raw_array_ensure_size(aingle_raw_array_t *array, size_t desired_count);

/**
 * Ensures that there is enough allocated space to store the given
 * number of elements in an aingle_raw_array_t.  If the array grows as a
 * result of this operation, we will fill in any newly allocated space
 * with 0 bytes.  If we can't allocate that much space, we return
 * ENOMEM.
 */

int
aingle_raw_array_ensure_size0(aingle_raw_array_t *array, size_t desired_count);

/**
 * Returns the number of elements in an aingle_raw_array_t.
 */

#define aingle_raw_array_size(array) ((array)->element_count)

/**
 * Returns the given element of an aingle_raw_array_t as a <code>void
 * *</code>.
 */

#define aingle_raw_array_get_raw(array, index) \
	((char *) (array)->data + (array)->element_size * index)

/**
 * Returns the given element of an aingle_raw_array_t, using element_type
 * as the type of the elements.  The result is *not* a pointer to the
 * element; you get the element itself.
 */

#define aingle_raw_array_get(array, element_type, index) \
	(((element_type *) (array)->data)[index])

/**
 * Appends a new element to an aingle_raw_array_t, expanding it if
 * necessary.  Returns a pointer to the new element, or NULL if we
 * needed to reallocate the array and couldn't.
 */

void *aingle_raw_array_append(aingle_raw_array_t *array);


/*---------------------------------------------------------------------
 * Maps
 */

/**
 * The type of the elements in a map's elements array.
 */

typedef struct aingle_raw_map_entry {
	const char  *key;
} aingle_raw_map_entry_t;

/**
 * A string-indexed map of fixed-size elements.
 */

typedef struct aingle_raw_map {
	aingle_raw_array_t  elements;
	void  *indices_by_key;
} aingle_raw_map_t;

/**
 * Initializes a new aingle_raw_map_t that you've allocated yourself.
 */

void aingle_raw_map_init(aingle_raw_map_t *map, size_t element_size);

/**
 * Finalizes an aingle_raw_map_t.
 */

void aingle_raw_map_done(aingle_raw_map_t *map);

/**
 * Clears an aingle_raw_map_t.
 */

void aingle_raw_map_clear(aingle_raw_map_t *map);

/**
 * Ensures that there is enough allocated space to store the given
 * number of elements in an aingle_raw_map_t.  If we can't allocate that
 * much space, we return ENOMEM.
 */

int
aingle_raw_map_ensure_size(aingle_raw_map_t *map, size_t desired_count);

/**
 * Returns the number of elements in an aingle_raw_map_t.
 */

#define aingle_raw_map_size(map)  aingle_raw_array_size(&((map)->elements))

/**
 * Returns the aingle_raw_map_entry_t for a given index.
 */

#define aingle_raw_get_entry(map, index) \
	((aingle_raw_map_entry_t *) \
	 aingle_raw_array_get_raw(&(map)->elements, index))

/**
 * Returns the given element of an aingle_raw_array_t as a <code>void
 * *</code>.  The indexes are assigned based on the order that the
 * elements are added to the map.
 */

#define aingle_raw_map_get_raw(map, index) \
	(aingle_raw_array_get_raw(&(map)->elements, index) + \
	 sizeof(aingle_raw_map_entry_t))

/**
 * Returns the element of an aingle_raw_map_t with the given numeric
 * index.  The indexes are assigned based on the order that the elements
 * are added to the map.
 */

#define aingle_raw_map_get_by_index(map, element_type, index) \
	(*((element_type *) aingle_raw_map_get_raw(map, index)))

/**
 * Returns the key of the element with the given numeric index.
 */

#define aingle_raw_map_get_key(map, index) \
	(aingle_raw_get_entry(map, index)->key)

/**
 * Returns the element of an aingle_raw_map_t with the given string key.
 * If the given element doesn't exist, returns NULL.  If @ref index
 * isn't NULL, it will be filled in with the index of the element.
 */

void *aingle_raw_map_get(const aingle_raw_map_t *map, const char *key,
		       size_t *index);

/**
 * Retrieves the element of an aingle_raw_map_t with the given string key,
 * creating it if necessary.  A pointer to the element is placed into
 * @ref element.  If @ref index isn't NULL, it will be filled in with
 * the index of the element.  We return 1 if the element is new; 0 if
 * it's not, and a negative error code if there was some problem.
 */

int aingle_raw_map_get_or_create(aingle_raw_map_t *map, const char *key,
			       void **element, size_t *index);


/*---------------------------------------------------------------------
 * Wrapped buffers
 */

/**
 * A pointer to an unmodifiable external memory region, along with
 * functions for freeing that buffer when it's no longer needed, and
 * copying it.
 */

typedef struct aingle_wrapped_buffer  aingle_wrapped_buffer_t;

struct aingle_wrapped_buffer {
	/** A pointer to the memory region */
	const void  *buf;

	/** The size of the memory region */
	size_t  size;

	/** Additional data needed by the methods below */
	void  *user_data;

	/**
	 * A function that will be called when the memory region is no
	 * longer needed.  This pointer can be NULL if nothing special
	 * needs to be done to free the buffer.
	 */
	void
	(*free)(aingle_wrapped_buffer_t *self);

	/**
	 * A function that makes a copy of a portion of a wrapped
	 * buffer.  This doesn't have to involve duplicating the memory
	 * region, but it should ensure that the free method can be
	 * safely called on both copies without producing any errors or
	 * memory corruption.  If this function is NULL, then we'll use
	 * a default implementation that calls @ref
	 * aingle_wrapped_buffer_new_copy.
	 */
	int
	(*copy)(aingle_wrapped_buffer_t *dest, const aingle_wrapped_buffer_t *src,
		size_t offset, size_t length);

	/**
	 * A function that "slices" a wrapped buffer, causing it to
	 * point at a subset of the existing buffer.  Usually, this just
	 * requires * updating the @ref buf and @ref size fields.  If
	 * you don't need to do anything other than this, this function
	 * pointer can be left @c NULL.  The function can assume that
	 * the @a offset and @a length parameters point to a valid
	 * subset of the existing wrapped buffer.
	 */
	int
	(*slice)(aingle_wrapped_buffer_t *self, size_t offset, size_t length);
};

/**
 * Free a wrapped buffer.
 */

#define aingle_wrapped_buffer_free(self) \
	do { \
		if ((self)->free != NULL) { \
			(self)->free((self)); \
		} \
	} while (0)

/**
 * A static initializer for an empty wrapped buffer.
 */

#define AINGLE_WRAPPED_BUFFER_EMPTY  { NULL, 0, NULL, NULL, NULL, NULL }

/**
 * Moves a wrapped buffer.  After returning, @a dest will wrap the
 * buffer that @a src used to point at, and @a src will be empty.
 */

void
aingle_wrapped_buffer_move(aingle_wrapped_buffer_t *dest,
			 aingle_wrapped_buffer_t *src);

/**
 * Copies a buffer.
 */

int
aingle_wrapped_buffer_copy(aingle_wrapped_buffer_t *dest,
			 const aingle_wrapped_buffer_t *src,
			 size_t offset, size_t length);

/**
 * Slices a buffer.
 */

int
aingle_wrapped_buffer_slice(aingle_wrapped_buffer_t *self,
			  size_t offset, size_t length);

/**
 * Creates a new wrapped buffer wrapping the given memory region.  You
 * have to ensure that buf stays around for as long as you need to new
 * wrapped buffer.  If you copy the wrapped buffer (using
 * aingle_wrapped_buffer_copy), this will create a copy of the data.
 * Additional copies will reuse this new copy.
 */

int
aingle_wrapped_buffer_new(aingle_wrapped_buffer_t *dest,
			const void *buf, size_t length);

/**
 * Creates a new wrapped buffer wrapping the given C string.
 */

#define aingle_wrapped_buffer_new_string(dest, str) \
    (aingle_wrapped_buffer_new((dest), (str), strlen((str))+1))

/**
 * Creates a new wrapped buffer containing a copy of the given memory
 * region.  This new copy will be reference counted; if you copy it
 * further (using aingle_wrapped_buffer_copy), the new copies will share a
 * single underlying buffer.
 */

int
aingle_wrapped_buffer_new_copy(aingle_wrapped_buffer_t *dest,
			     const void *buf, size_t length);

/**
 * Creates a new wrapped buffer containing a copy of the given C string.
 */

#define aingle_wrapped_buffer_new_string_copy(dest, str) \
    (aingle_wrapped_buffer_new_copy((dest), (str), strlen((str))+1))


/*---------------------------------------------------------------------
 * Strings
 */

/**
 * A resizable buffer for storing strings and bytes values.
 */

typedef struct aingle_raw_string {
	aingle_wrapped_buffer_t  wrapped;
} aingle_raw_string_t;

/**
 * Initializes an aingle_raw_string_t that you've allocated yourself.
 */

void aingle_raw_string_init(aingle_raw_string_t *str);

/**
 * Finalizes an aingle_raw_string_t.
 */

void aingle_raw_string_done(aingle_raw_string_t *str);

/**
 * Returns the length of the data stored in an aingle_raw_string_t.  If
 * the buffer contains a C string, this length includes the NUL
 * terminator.
 */

#define aingle_raw_string_length(str)  ((str)->wrapped.size)

/**
 * Returns a pointer to the data stored in an aingle_raw_string_t.
 */

#define aingle_raw_string_get(str)  ((str)->wrapped.buf)

/**
 * Fills an aingle_raw_string_t with a copy of the given buffer.
 */

void aingle_raw_string_set_length(aingle_raw_string_t *str,
				const void *src,
				size_t length);

/**
 * Fills an aingle_raw_string_t with a copy of the given C string.
 */

void aingle_raw_string_set(aingle_raw_string_t *str, const char *src);

/**
 * Appends the given C string to an aingle_raw_string_t.
 */

void aingle_raw_string_append(aingle_raw_string_t *str, const char *src);

/**
 * Appends the given C string to an aingle_raw_string_t, using the
 * provided length instead of calling strlen(src).
 */

void aingle_raw_string_append_length(aingle_raw_string_t *str,
				   const void *src,
				   size_t length);
/**
 * Gives control of a buffer to an aingle_raw_string_t.
 */

void
aingle_raw_string_give(aingle_raw_string_t *str,
		     aingle_wrapped_buffer_t *src);

/**
 * Returns an aingle_wrapped_buffer_t for the content of the string,
 * ideally without copying it.
 */

int
aingle_raw_string_grab(const aingle_raw_string_t *str,
		     aingle_wrapped_buffer_t *dest);

/**
 * Clears an aingle_raw_string_t.
 */

void aingle_raw_string_clear(aingle_raw_string_t *str);


/**
 * Tests two aingle_raw_string_t instances for equality.
 */

int aingle_raw_string_equals(const aingle_raw_string_t *str1,
			   const aingle_raw_string_t *str2);


/*---------------------------------------------------------------------
 * Memoization
 */

/**
 * A specialized map that can be used to memoize the results of a
 * function.  The API allows you to use two keys as the memoization
 * keys; if you only need one key, just use NULL for the second key.
 * The result of the function should be a single pointer, or an integer
 * that can be cast into a pointer (i.e., an intptr_t).
 */

typedef struct aingle_memoize {
	void  *cache;
} aingle_memoize_t;

/**
 * Initialize an aingle_memoize_t that you've allocated for yourself.
 */

void
aingle_memoize_init(aingle_memoize_t *mem);

/**
 * Finalizes an aingle_memoize_t.
 */

void
aingle_memoize_done(aingle_memoize_t *mem);

/**
 * Search for a cached value in an aingle_memoize_t.  Returns a boolean
 * indicating whether there's a value in the cache for the given keys.
 * If there is, the cached result is placed into @ref result.
 */

int
aingle_memoize_get(aingle_memoize_t *mem,
		 void *key1, void *key2,
		 void **result);

/**
 * Stores a new cached value into an aingle_memoize_t, overwriting it if
 * necessary.
 */

void
aingle_memoize_set(aingle_memoize_t *mem,
		 void *key1, void *key2,
		 void *result);

/**
 * Removes any cached value for the given key from an aingle_memoize_t.
 */

void
aingle_memoize_delete(aingle_memoize_t *mem, void *key1, void *key2);

CLOSE_EXTERN
#endif
