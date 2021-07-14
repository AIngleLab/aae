/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.	 You may obtain a copy of the License at
 *
 * 
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <aingle/platform.h>
#include <stdlib.h>
#include <string.h>

#include "aingle/allocation.h"
#include "aingle/consumer.h"
#include "aingle/data.h"
#include "aingle/errors.h"
#include "aingle/legacy.h"
#include "aingle/schema.h"
#include "aingle_private.h"
#include "st.h"


#if !defined(DEBUG_RESOLVER)
#define DEBUG_RESOLVER 0
#endif

#if DEBUG_RESOLVER
#include <stdio.h>
#define debug(...) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#else
#define debug(...) /* no debug output */
#endif


typedef struct aingle_resolver_t  aingle_resolver_t;

struct aingle_resolver_t {
	aingle_consumer_t  parent;

	/* The reader schema for this resolver. */
	aingle_schema_t  rschema;

	/* An array of any child resolvers needed for the subschemas of
	 * wschema */
	aingle_consumer_t  **child_resolvers;

	/* If the reader and writer schemas are records, this field
	 * contains a mapping from writer field indices to reader field
	 * indices. */
	int  *index_mapping;

	/* The number of elements in the child_resolvers and
	 * index_mapping arrays. */
	size_t  num_children;

	/* If the reader schema is a union, but the writer schema is
	 * not, this field indicates which branch of the reader union
	 * should be selected. */
	int  reader_union_branch;
};


/**
 * Frees a resolver object, while ensuring that all of the resolvers in
 * a graph of resolvers is only freed once.
 */

static void
aingle_resolver_free_cycles(aingle_consumer_t *consumer, st_table *freeing)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;

	/*
	 * First check if we've already started freeing this resolver.
	 */

	if (st_lookup(freeing, (st_data_t) resolver, NULL)) {
		return;
	}

	/*
	 * Otherwise add this resolver to the freeing set, and then
	 * actually free the thing.
	 */

	st_insert(freeing, (st_data_t) resolver, (st_data_t) NULL);

	aingle_schema_decref(resolver->parent.schema);
	aingle_schema_decref(resolver->rschema);
	if (resolver->child_resolvers) {
		unsigned int  i;
		for (i = 0; i < resolver->num_children; i++) {
			aingle_consumer_t  *child = resolver->child_resolvers[i];
			if (child) {
				aingle_resolver_free_cycles(child, freeing);
			}
		}
		aingle_free(resolver->child_resolvers,
			  sizeof(aingle_resolver_t *) * resolver->num_children);
	}
	if (resolver->index_mapping) {
		aingle_free(resolver->index_mapping,
			  sizeof(int) * resolver->num_children);
	}
	aingle_freet(aingle_resolver_t, resolver);
}


static void
aingle_resolver_free(aingle_consumer_t *consumer)
{
	st_table  *freeing = st_init_numtable();
	aingle_resolver_free_cycles(consumer, freeing);
	st_free_table(freeing);
}

/**
 * Create a new aingle_resolver_t instance.  You must fill in the callback
 * pointers that are appropriate for the writer schema after this
 * function returns.
 */

static aingle_resolver_t *
aingle_resolver_create(aingle_schema_t wschema,
		     aingle_schema_t rschema)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) aingle_new(aingle_resolver_t);
	memset(resolver, 0, sizeof(aingle_resolver_t));

	resolver->parent.free = aingle_resolver_free;
	resolver->parent.schema = aingle_schema_incref(wschema);
	resolver->rschema = aingle_schema_incref(rschema);
	resolver->reader_union_branch = -1;
	return resolver;
}


static aingle_datum_t
aingle_resolver_get_real_dest(aingle_resolver_t *resolver, aingle_datum_t dest)
{
	if (resolver->reader_union_branch < 0) {
		/*
		 * The reader schema isn't a union, so use the dest
		 * field as-is.
		 */

		return dest;
	}

	debug("Retrieving union branch %d for %s value",
	      resolver->reader_union_branch,
	      aingle_schema_type_name(resolver->parent.schema));

	aingle_datum_t  branch = NULL;
	aingle_union_set_discriminant
	    (dest, resolver->reader_union_branch, &branch);
	return branch;
}


#define skip_links(schema)					\
	while (is_aingle_link(schema)) {				\
		schema = aingle_schema_link_target(schema);	\
	}


/*-----------------------------------------------------------------------
 * Memoized resolvers
 */

static aingle_consumer_t *
aingle_resolver_new_memoized(aingle_memoize_t *mem,
			   aingle_schema_t wschema, aingle_schema_t rschema);


/*-----------------------------------------------------------------------
 * Reader unions
 */

/*
 * For each AIngle type, we have to check whether the reader schema on its
 * own is compatible, and whether the reader is a union that contains a
 * compatible type.  The macros in this section help us perform both of
 * these checks with less code.
 */


/**
 * A helper macro that handles the case where neither writer nor reader
 * are unions.  Uses @ref check_func to see if the two schemas are
 * compatible.
 */

#define check_non_union(saved, wschema, rschema, check_func)	\
do {								\
	aingle_resolver_t  *self = NULL;				\
	int  rc = check_func(saved, &self, wschema, rschema,	\
			     rschema);				\
	if (self) {						\
		debug("Non-union schemas %s (writer) "		\
		      "and %s (reader) match",			\
		      aingle_schema_type_name(wschema),		\
		      aingle_schema_type_name(rschema));		\
								\
		self->reader_union_branch = -1;			\
		return &self->parent;				\
        }							\
								\
        if (rc) {						\
		return NULL;					\
	}							\
} while (0)


/**
 * Helper macro that handles the case where the reader is a union, and
 * the writer is not.  Checks each branch of the reader union schema,
 * looking for the first branch that is compatible with the writer
 * schema.  The @ref check_func argument should be a function that can
 * check the compatiblity of each branch schema.
 */

#define check_reader_union(saved, wschema, rschema, check_func)		\
do {									\
	if (!is_aingle_union(rschema)) {					\
		break;							\
	}								\
									\
	debug("Checking reader union schema");				\
	size_t  num_branches = aingle_schema_union_size(rschema);		\
	unsigned int  i;						\
									\
	for (i = 0; i < num_branches; i++) {				\
		aingle_schema_t  branch_schema =				\
		    aingle_schema_union_branch(rschema, i);		\
		skip_links(branch_schema);				\
		aingle_resolver_t  *self = NULL;				\
		int  rc = check_func(saved, &self,			\
				     wschema, branch_schema,		\
				     rschema);				\
		if (self) {						\
			debug("Reader union branch %d (%s) "		\
			      "and writer %s match",			\
			      i, aingle_schema_type_name(branch_schema),	\
			      aingle_schema_type_name(wschema));		\
			self->reader_union_branch = i;			\
			return &self->parent;				\
		} else {						\
			debug("Reader union branch %d (%s) "		\
			      "doesn't match",				\
			      i, aingle_schema_type_name(branch_schema));	\
		}							\
									\
		if (rc) {						\
			return NULL;					\
		}							\
	}								\
									\
	debug("No reader union branches match");			\
} while (0)

/**
 * A helper macro that defines wraps together check_non_union and
 * check_reader_union for a simple (non-union) writer schema type.
 */

#define check_simple_writer(saved, wschema, rschema, type_name)		\
do {									\
	check_non_union(saved, wschema, rschema, try_##type_name);	\
	check_reader_union(saved, wschema, rschema, try_##type_name);	\
	debug("Writer %s doesn't match reader %s",			\
	      aingle_schema_type_name(wschema),				\
	      aingle_schema_type_name(rschema));				\
	aingle_set_error("Cannot store " #type_name " into %s",		\
		       aingle_schema_type_name(rschema));			\
	return NULL;							\
} while (0)


/*-----------------------------------------------------------------------
 * primitives
 */

static int
aingle_resolver_boolean_value(aingle_consumer_t *consumer, int value,
			    void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %s into %p", value? "TRUE": "FALSE", dest);
	return aingle_boolean_set(dest, value);
}

static int
try_boolean(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	    aingle_schema_t wschema, aingle_schema_t rschema,
	    aingle_schema_t root_rschema)
{
	if (is_aingle_boolean(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.boolean_value = aingle_resolver_boolean_value;
	}
	return 0;
}


static void
free_bytes(void *ptr, size_t sz)
{
	/*
	 * The binary encoder class allocates bytes values with an extra
	 * byte, so that they're NUL terminated.
	 */
	aingle_free(ptr, sz+1);
}

static int
aingle_resolver_bytes_value(aingle_consumer_t *consumer,
			  const void *value, size_t value_len,
			  void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRIsz " bytes into %p", value_len, dest);
	return aingle_givebytes_set(dest, (const char *) value, value_len, free_bytes);
}

static int
try_bytes(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	  aingle_schema_t wschema, aingle_schema_t rschema,
	  aingle_schema_t root_rschema)
{
	if (is_aingle_bytes(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.bytes_value = aingle_resolver_bytes_value;
	}
	return 0;
}


static int
aingle_resolver_double_value(aingle_consumer_t *consumer, double value,
			   void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %le into %p", value, dest);
	return aingle_double_set(dest, value);
}

static int
try_double(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	   aingle_schema_t wschema, aingle_schema_t rschema,
	   aingle_schema_t root_rschema)
{
	if (is_aingle_double(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.double_value = aingle_resolver_double_value;
	}
	return 0;
}


static int
aingle_resolver_float_value(aingle_consumer_t *consumer, float value,
			  void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %e into %p", value, dest);
	return aingle_float_set(dest, value);
}

static int
aingle_resolver_float_double_value(aingle_consumer_t *consumer, float value,
				 void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %e into %p (promoting float to double)", value, dest);
	return aingle_double_set(dest, value);
}

static int
try_float(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	  aingle_schema_t wschema, aingle_schema_t rschema,
	  aingle_schema_t root_rschema)
{
	if (is_aingle_float(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.float_value = aingle_resolver_float_value;
	}
	else if (is_aingle_double(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.float_value = aingle_resolver_float_double_value;
	}
	return 0;
}


static int
aingle_resolver_int_value(aingle_consumer_t *consumer, int32_t value,
			void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRId32 " into %p", value, dest);
	return aingle_int32_set(dest, value);
}

static int
aingle_resolver_int_long_value(aingle_consumer_t *consumer, int32_t value,
			     void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRId32 " into %p (promoting int to long)", value, dest);
	return aingle_int64_set(dest, value);
}

static int
aingle_resolver_int_double_value(aingle_consumer_t *consumer, int32_t value,
			       void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRId32 " into %p (promoting int to double)", value, dest);
	return aingle_double_set(dest, value);
}

static int
aingle_resolver_int_float_value(aingle_consumer_t *consumer, int32_t value,
			      void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRId32 " into %p (promoting int to float)", value, dest);
	return aingle_float_set(dest, (const float) value);
}

static int
try_int(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	aingle_schema_t wschema, aingle_schema_t rschema,
	aingle_schema_t root_rschema)
{
	if (is_aingle_int32(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.int_value = aingle_resolver_int_value;
	}
	else if (is_aingle_int64(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.int_value = aingle_resolver_int_long_value;
	}
	else if (is_aingle_double(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.int_value = aingle_resolver_int_double_value;
	}
	else if (is_aingle_float(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.int_value = aingle_resolver_int_float_value;
	}
	return 0;
}


static int
aingle_resolver_long_value(aingle_consumer_t *consumer, int64_t value,
			 void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRId64 " into %p", value, dest);
	return aingle_int64_set(dest, value);
}

static int
aingle_resolver_long_float_value(aingle_consumer_t *consumer, int64_t value,
			       void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRId64 " into %p (promoting long to float)", value, dest);
	return aingle_float_set(dest, (const float) value);
}

static int
aingle_resolver_long_double_value(aingle_consumer_t *consumer, int64_t value,
				void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing %" PRId64 " into %p (promoting long to double)", value, dest);
	return aingle_double_set(dest, (const double) value);
}

static int
try_long(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	 aingle_schema_t wschema, aingle_schema_t rschema,
	 aingle_schema_t root_rschema)
{
	if (is_aingle_int64(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.long_value = aingle_resolver_long_value;
	}
	else if (is_aingle_double(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.long_value = aingle_resolver_long_double_value;
	}
	else if (is_aingle_float(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.long_value = aingle_resolver_long_float_value;
	}
	return 0;
}


static int
aingle_resolver_null_value(aingle_consumer_t *consumer, void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);

	AINGLE_UNUSED(dest);
	debug("Storing null into %p", dest);
	return 0;
}

static int
try_null(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	 aingle_schema_t wschema, aingle_schema_t rschema,
	 aingle_schema_t root_rschema)
{
	if (is_aingle_null(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.null_value = aingle_resolver_null_value;
	}
	return 0;
}


static int
aingle_resolver_string_value(aingle_consumer_t *consumer,
			   const void *value, size_t value_len,
			   void *user_data)
{
	AINGLE_UNUSED(value_len);
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing \"%s\" into %p", (const char *) value, dest);
	return aingle_givestring_set(dest, (const char *) value, aingle_alloc_free_func);
}

static int
try_string(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	   aingle_schema_t wschema, aingle_schema_t rschema,
	   aingle_schema_t root_rschema)
{
	if (is_aingle_string(rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.string_value = aingle_resolver_string_value;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * arrays
 */

static int
aingle_resolver_array_start_block(aingle_consumer_t *consumer,
				int is_first_block,
				unsigned int block_count,
				void *user_data)
{
	if (is_first_block) {
		aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
		aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
		aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
		AINGLE_UNUSED(dest);

		debug("Starting array %p", dest);
	}

	AINGLE_UNUSED(block_count);
	return 0;
}

static int
aingle_resolver_array_element(aingle_consumer_t *consumer,
			    unsigned int index,
			    aingle_consumer_t **element_consumer,
			    void **element_user_data,
			    void *user_data)
{
	AINGLE_UNUSED(index);

	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Adding element to array %p", dest);

	/*
	 * Allocate a new element datum and add it to the array.
	 */

	aingle_schema_t  array_schema = aingle_datum_get_schema(dest);
	aingle_schema_t  item_schema = aingle_schema_array_items(array_schema);
	aingle_datum_t  element = aingle_datum_from_schema(item_schema);
	aingle_array_append_datum(dest, element);
	aingle_datum_decref(element);

	/*
	 * Return the consumer that we allocated to process the array's
	 * children.
	 */

	*element_consumer = resolver->child_resolvers[0];
	*element_user_data = element;
	return 0;
}

static int
try_array(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	  aingle_schema_t wschema, aingle_schema_t rschema,
	  aingle_schema_t root_rschema)
{
	/*
	 * First verify that the reader is an array.
	 */

	if (!is_aingle_array(rschema)) {
		return 0;
	}

	/*
	 * Array schemas have to have compatible element schemas to be
	 * compatible themselves.  Try to create an aingle_resolver_t to
	 * check the compatibility.
	 */

	*resolver = aingle_resolver_create(wschema, root_rschema);
	aingle_memoize_set(mem, wschema, root_rschema, *resolver);

	aingle_schema_t  witems = aingle_schema_array_items(wschema);
	aingle_schema_t  ritems = aingle_schema_array_items(rschema);

	aingle_consumer_t  *item_consumer =
	    aingle_resolver_new_memoized(mem, witems, ritems);
	if (!item_consumer) {
		aingle_memoize_delete(mem, wschema, root_rschema);
		aingle_consumer_free(&(*resolver)->parent);
		aingle_prefix_error("Array values aren't compatible: ");
		return EINVAL;
	}

	/*
	 * The two schemas are compatible, so go ahead and create a
	 * GaingleResolver for the array.  Store the item schema's
	 * resolver into the child_resolvers field.
	 */

	(*resolver)->num_children = 1;
	(*resolver)->child_resolvers = (aingle_consumer_t **) aingle_calloc(1, sizeof(aingle_consumer_t *));
	(*resolver)->child_resolvers[0] = item_consumer;
	(*resolver)->parent.array_start_block = aingle_resolver_array_start_block;
	(*resolver)->parent.array_element = aingle_resolver_array_element;

	return 0;
}


/*-----------------------------------------------------------------------
 * enums
 */

static int
aingle_resolver_enum_value(aingle_consumer_t *consumer, int value,
			 void *user_data)
{
	AINGLE_UNUSED(value);

	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);

	const char  *symbol_name = aingle_schema_enum_get(resolver->parent.schema, value);
	debug("Storing symbol %s into %p", symbol_name, dest);
	return aingle_enum_set_name(dest, symbol_name);
}

static int
try_enum(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	 aingle_schema_t wschema, aingle_schema_t rschema,
	 aingle_schema_t root_rschema)
{
	/*
	 * Enum schemas have to have the same name — but not the same
	 * list of symbols — to be compatible.
	 */

	if (is_aingle_enum(rschema)) {
		const char  *wname = aingle_schema_name(wschema);
		const char  *rname = aingle_schema_name(rschema);

		if (!strcmp(wname, rname)) {
			*resolver = aingle_resolver_create(wschema, root_rschema);
			aingle_memoize_set(mem, wschema, root_rschema, *resolver);
			(*resolver)->parent.enum_value = aingle_resolver_enum_value;
		}
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * fixed
 */

static int
aingle_resolver_fixed_value(aingle_consumer_t *consumer,
			  const void *value, size_t value_len,
			  void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Storing (fixed) %" PRIsz " bytes into %p", value_len, dest);
	return aingle_givefixed_set(dest, (const char *) value, value_len, aingle_alloc_free_func);
}

static int
try_fixed(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	  aingle_schema_t wschema, aingle_schema_t rschema,
	  aingle_schema_t root_rschema)
{
	/*
	 * Fixed schemas need the same name and size to be compatible.
	 */

	if (aingle_schema_equal(wschema, rschema)) {
		*resolver = aingle_resolver_create(wschema, root_rschema);
		aingle_memoize_set(mem, wschema, root_rschema, *resolver);
		(*resolver)->parent.fixed_value = aingle_resolver_fixed_value;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * maps
 */

static int
aingle_resolver_map_start_block(aingle_consumer_t *consumer,
			      int is_first_block,
			      unsigned int block_count,
			      void *user_data)
{
	if (is_first_block) {
		aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
		aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
		aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
		AINGLE_UNUSED(dest);

		debug("Starting map %p", dest);
	}

	AINGLE_UNUSED(block_count);
	return 0;
}

static int
aingle_resolver_map_element(aingle_consumer_t *consumer,
			  unsigned int index,
			  const char *key,
			  aingle_consumer_t **value_consumer,
			  void **value_user_data,
			  void *user_data)
{
	AINGLE_UNUSED(index);

	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	debug("Adding element to map %p", dest);

	/*
	 * Allocate a new element datum and add it to the map.
	 */

	aingle_schema_t  map_schema = aingle_datum_get_schema(dest);
	aingle_schema_t  value_schema = aingle_schema_map_values(map_schema);
	aingle_datum_t  value = aingle_datum_from_schema(value_schema);
	aingle_map_set(dest, key, value);
	aingle_datum_decref(value);

	/*
	 * Return the consumer that we allocated to process the map's
	 * children.
	 */

	*value_consumer = resolver->child_resolvers[0];
	*value_user_data = value;
	return 0;
}

static int
try_map(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	aingle_schema_t wschema, aingle_schema_t rschema,
	aingle_schema_t root_rschema)
{
	/*
	 * First verify that the reader is an map.
	 */

	if (!is_aingle_map(rschema)) {
		return 0;
	}

	/*
	 * Array schemas have to have compatible element schemas to be
	 * compatible themselves.  Try to create an aingle_resolver_t to
	 * check the compatibility.
	 */

	*resolver = aingle_resolver_create(wschema, root_rschema);
	aingle_memoize_set(mem, wschema, root_rschema, *resolver);

	aingle_schema_t  wvalues = aingle_schema_map_values(wschema);
	aingle_schema_t  rvalues = aingle_schema_map_values(rschema);

	aingle_consumer_t  *value_consumer =
	    aingle_resolver_new_memoized(mem, wvalues, rvalues);
	if (!value_consumer) {
		aingle_memoize_delete(mem, wschema, root_rschema);
		aingle_consumer_free(&(*resolver)->parent);
		aingle_prefix_error("Map values aren't compatible: ");
		return EINVAL;
	}

	/*
	 * The two schemas are compatible, so go ahead and create a
	 * GaingleResolver for the map.  Store the value schema's
	 * resolver into the child_resolvers field.
	 */

	(*resolver)->num_children = 1;
	(*resolver)->child_resolvers = (aingle_consumer_t **) aingle_calloc(1, sizeof(aingle_consumer_t *));
	(*resolver)->child_resolvers[0] = value_consumer;
	(*resolver)->parent.map_start_block = aingle_resolver_map_start_block;
	(*resolver)->parent.map_element = aingle_resolver_map_element;

	return 0;
}


/*-----------------------------------------------------------------------
 * records
 */

static int
aingle_resolver_record_start(aingle_consumer_t *consumer,
			   void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);
	AINGLE_UNUSED(dest);

	debug("Starting record at %p", dest);

	/*
	 * TODO: Eventually, we'll fill in default values for the extra
	 * reader fields here.
	 */

	return 0;
}

static int
aingle_resolver_record_field(aingle_consumer_t *consumer,
			   unsigned int index,
			   aingle_consumer_t **field_consumer,
			   void **field_user_data,
			   void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;
	aingle_datum_t  ud_dest = (aingle_datum_t) user_data;
	aingle_datum_t  dest = aingle_resolver_get_real_dest(resolver, ud_dest);

	const char  *field_name =
	    aingle_schema_record_field_name(consumer->schema, index);

	/*
	 * Grab the resolver for this field of the writer record.  If
	 * it's NULL, this this field doesn't exist in the reader
	 * record, and should be skipped.
	 */

	debug("Retrieving resolver for writer field %i (%s)",
	      index, field_name);

	if (!resolver->child_resolvers[index]) {
		debug("Reader doesn't have field %s, skipping", field_name);
		return 0;
	}

	/*
	 * TODO: Once we can retrieve record fields by index (quickly),
	 * use the index_mapping.
	 */

	aingle_datum_t  field = NULL;
	aingle_record_get(dest, field_name, &field);

	*field_consumer = resolver->child_resolvers[index];
	*field_user_data = field;
	return 0;
}

static int
try_record(aingle_memoize_t *mem, aingle_resolver_t **resolver,
	   aingle_schema_t wschema, aingle_schema_t rschema,
	   aingle_schema_t root_rschema)
{
	/*
	 * First verify that the reader is also a record, and has the
	 * same name as the writer.
	 */

	if (!is_aingle_record(rschema)) {
		return 0;
	}

	const char  *wname = aingle_schema_name(wschema);
	const char  *rname = aingle_schema_name(rschema);

	if (strcmp(wname, rname)) {
		return 0;
	}

	/*
	 * Categorize the fields in the record schemas.  Fields that are
	 * only in the writer are ignored.  Fields that are only in the
	 * reader raise a schema mismatch error, unless the field has a
	 * default value.  Fields that are in both are resolved
	 * recursively.
	 *
	 * The child_resolver array will contain an aingle_resolver_t for
	 * each field in the writer schema.  To build this array, we
	 * loop through the fields of the reader schema.  If that field
	 * is also in the writer schema, we resolve them recursively,
	 * and store the resolver into the array.  If the field isn't in
	 * the writer schema, we raise an error.  (TODO: Eventually,
	 * we'll handle default values here.)  After this loop finishes,
	 * any NULLs in the child_resolver array will represent fields
	 * in the writer but not the reader; these fields will be
	 * skipped when processing the input.
	 */

	*resolver = aingle_resolver_create(wschema, root_rschema);
	aingle_memoize_set(mem, wschema, root_rschema, *resolver);

	size_t  wfields = aingle_schema_record_size(wschema);
	size_t  rfields = aingle_schema_record_size(rschema);

	debug("Checking writer record schema %s", wname);

	aingle_consumer_t  **child_resolvers =
	    (aingle_consumer_t **) aingle_calloc(wfields, sizeof(aingle_consumer_t *));
	int  *index_mapping = (int *) aingle_calloc(wfields, sizeof(int));

	unsigned int  ri;
	for (ri = 0; ri < rfields; ri++) {
		aingle_schema_t  rfield =
		    aingle_schema_record_field_get_by_index(rschema, ri);
		const char  *field_name =
		    aingle_schema_record_field_name(rschema, ri);

		debug("Resolving reader record field %u (%s)", ri, field_name);

		/*
		 * See if this field is also in the writer schema.
		 */

		int  wi = aingle_schema_record_field_get_index(wschema, field_name);

		if (wi == -1) {
			/*
			 * This field isn't in the writer schema —
			 * that's an error!  TODO: Handle default
			 * values!
			 */

			debug("Field %s isn't in writer", field_name);
			aingle_set_error("Reader field %s doesn't appear in writer",
				       field_name);
			goto error;
		}

		/*
		 * Try to recursively resolve the schemas for this
		 * field.  If they're not compatible, that's an error.
		 */

		aingle_schema_t  wfield =
		    aingle_schema_record_field_get_by_index(wschema, wi);
		aingle_consumer_t  *field_resolver =
		    aingle_resolver_new_memoized(mem, wfield, rfield);

		if (!field_resolver) {
			aingle_prefix_error("Field %s isn't compatible: ", field_name);
			goto error;
		}

		/*
		 * Save the details for this field.
		 */

		debug("Found match for field %s (%u in reader, %d in writer)",
		      field_name, ri, wi);
		child_resolvers[wi] = field_resolver;
		index_mapping[wi] = ri;
	}

	/*
	 * We might not have found matches for all of the writer fields,
	 * but that's okay — any extras will be ignored.
	 */

	(*resolver)->num_children = wfields;
	(*resolver)->child_resolvers = child_resolvers;
	(*resolver)->index_mapping = index_mapping;
	(*resolver)->parent.record_start = aingle_resolver_record_start;
	(*resolver)->parent.record_field = aingle_resolver_record_field;
	return 0;

error:
	/*
	 * Clean up any consumer we might have already created.
	 */

	aingle_memoize_delete(mem, wschema, root_rschema);
	aingle_consumer_free(&(*resolver)->parent);

	{
		unsigned int  i;
		for (i = 0; i < wfields; i++) {
			if (child_resolvers[i]) {
				aingle_consumer_free(child_resolvers[i]);
			}
		}
	}

	aingle_free(child_resolvers, wfields * sizeof(aingle_consumer_t *));
	aingle_free(index_mapping, wfields * sizeof(int));
	return EINVAL;
}


/*-----------------------------------------------------------------------
 * union
 */

static int
aingle_resolver_union_branch(aingle_consumer_t *consumer,
			   unsigned int discriminant,
			   aingle_consumer_t **branch_consumer,
			   void **branch_user_data,
			   void *user_data)
{
	aingle_resolver_t  *resolver = (aingle_resolver_t *) consumer;

	/*
	 * Grab the resolver for this branch of the writer union.  If
	 * it's NULL, then this branch is incompatible with the reader.
	 */

	debug("Retrieving resolver for writer branch %u", discriminant);

	if (!resolver->child_resolvers[discriminant]) {
		aingle_set_error("Writer union branch %u is incompatible "
			       "with reader schema \"%s\"",
			       discriminant, aingle_schema_type_name(resolver->rschema));
		return EINVAL;
	}

	/*
	 * Return the branch's resolver.
	 */

	*branch_consumer = resolver->child_resolvers[discriminant];
	*branch_user_data = user_data;
	return 0;
}

static aingle_consumer_t *
try_union(aingle_memoize_t *mem, aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * For a writer union, we recursively try to resolve each branch
	 * against the reader schema.  This will work correctly whether
	 * or not the reader is also a union — if the reader is a union,
	 * then we'll resolve each (non-union) writer branch against the
	 * reader union, which will be checked in our calls to
	 * check_simple_writer below.  The net result is that we might
	 * end up trying every combination of writer and reader
	 * branches, when looking for compatible schemas.
	 *
	 * Regardless of what the reader schema is, for each writer
	 * branch, we stash away the recursive aingle_resolver_t into the
	 * child_resolvers array.  A NULL entry in this array means that
	 * that branch isn't compatible with the reader.  This isn't an
	 * immediate schema resolution error, since we allow
	 * incompatible branches in the types as long as that branch
	 * never appears in the actual data.  We only return an error if
	 * there are *no* branches that are compatible.
	 */

	size_t  num_branches = aingle_schema_union_size(wschema);
	debug("Checking %" PRIsz "-branch writer union schema", num_branches);

	aingle_resolver_t  *resolver = aingle_resolver_create(wschema, rschema);
	aingle_memoize_set(mem, wschema, rschema, resolver);

	aingle_consumer_t  **child_resolvers =
	    (aingle_consumer_t **) aingle_calloc(num_branches, sizeof(aingle_consumer_t *));
	int  some_branch_compatible = 0;

	unsigned int  i;
	for (i = 0; i < num_branches; i++) {
		aingle_schema_t  branch_schema =
		    aingle_schema_union_branch(wschema, i);

		debug("Resolving writer union branch %u (%s)",
		      i, aingle_schema_type_name(branch_schema));

		/*
		 * Try to recursively resolve this branch of the writer
		 * union.  Don't raise an error if this fails — it's
		 * okay for some of the branches to not be compatible
		 * with the reader, as long as those branches never
		 * appear in the input.
		 */

		child_resolvers[i] =
		    aingle_resolver_new_memoized(mem, branch_schema, rschema);
		if (child_resolvers[i]) {
			debug("Found match for writer union branch %u", i);
			some_branch_compatible = 1;
		} else {
			debug("No match for writer union branch %u", i);
		}
	}

	/*
	 * As long as there's at least one branch that's compatible with
	 * the reader, then we consider this schema resolution a
	 * success.
	 */

	if (!some_branch_compatible) {
		debug("No writer union branches match");
		aingle_set_error("No branches in the writer are compatible "
			       "with reader schema %s",
			       aingle_schema_type_name(rschema));
		goto error;
	}

	resolver->num_children = num_branches;
	resolver->child_resolvers = child_resolvers;
	resolver->parent.union_branch = aingle_resolver_union_branch;
	return &resolver->parent;

error:
	/*
	 * Clean up any consumer we might have already created.
	 */

	aingle_memoize_delete(mem, wschema, rschema);
	aingle_consumer_free(&resolver->parent);

	for (i = 0; i < num_branches; i++) {
		if (child_resolvers[i]) {
			aingle_consumer_free(child_resolvers[i]);
		}
	}

	aingle_free(child_resolvers, num_branches * sizeof(aingle_consumer_t *));
	return NULL;
}


/*-----------------------------------------------------------------------
 * schema type dispatcher
 */

static aingle_consumer_t *
aingle_resolver_new_memoized(aingle_memoize_t *mem,
			   aingle_schema_t wschema, aingle_schema_t rschema)
{
	check_param(NULL, is_aingle_schema(wschema), "writer schema");
	check_param(NULL, is_aingle_schema(rschema), "reader schema");

	skip_links(wschema);
	skip_links(rschema);

	/*
	 * First see if we've already matched these two schemas.  If so,
	 * just return that resolver.
	 */

	aingle_resolver_t  *saved = NULL;
	if (aingle_memoize_get(mem, wschema, rschema, (void **) &saved)) {
		debug("Already resolved %s and %s",
		      aingle_schema_type_name(wschema),
		      aingle_schema_type_name(rschema));
		return &saved->parent;
	}

	/*
	 * Otherwise we have some work to do.
	 */

	switch (aingle_typeof(wschema))
	{
		case AINGLE_BOOLEAN:
			check_simple_writer(mem, wschema, rschema, boolean);
			return NULL;

		case AINGLE_BYTES:
			check_simple_writer(mem, wschema, rschema, bytes);
			return NULL;

		case AINGLE_DOUBLE:
			check_simple_writer(mem, wschema, rschema, double);
			return NULL;

		case AINGLE_FLOAT:
			check_simple_writer(mem, wschema, rschema, float);
			return NULL;

		case AINGLE_INT32:
			check_simple_writer(mem, wschema, rschema, int);
			return NULL;

		case AINGLE_INT64:
			check_simple_writer(mem, wschema, rschema, long);
			return NULL;

		case AINGLE_NULL:
			check_simple_writer(mem, wschema, rschema, null);
			return NULL;

		case AINGLE_STRING:
			check_simple_writer(mem, wschema, rschema, string);
			return NULL;

		case AINGLE_ARRAY:
			check_simple_writer(mem, wschema, rschema, array);
			return NULL;

		case AINGLE_ENUM:
			check_simple_writer(mem, wschema, rschema, enum);
			return NULL;

		case AINGLE_FIXED:
			check_simple_writer(mem, wschema, rschema, fixed);
			return NULL;

		case AINGLE_MAP:
			check_simple_writer(mem, wschema, rschema, map);
			return NULL;

		case AINGLE_RECORD:
			check_simple_writer(mem, wschema, rschema, record);
			return NULL;

		case AINGLE_UNION:
			return try_union(mem, wschema, rschema);

		default:
			aingle_set_error("Unknown schema type");
			return NULL;
	}

	return NULL;
}


aingle_consumer_t *
aingle_resolver_new(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_memoize_t  mem;
	aingle_memoize_init(&mem);
	aingle_consumer_t  *result =
	    aingle_resolver_new_memoized(&mem, wschema, rschema);
	aingle_memoize_done(&mem);
	return result;
}
