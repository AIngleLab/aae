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

#include "aingle_private.h"
#include "aingle/allocation.h"
#include "aingle/basics.h"
#include "aingle/data.h"
#include "aingle/errors.h"
#include "aingle/refcount.h"
#include "aingle/resolver.h"
#include "aingle/schema.h"
#include "aingle/value.h"
#include "st.h"

#ifndef AINGLE_RESOLVER_DEBUG
#define AINGLE_RESOLVER_DEBUG 0
#endif

#if AINGLE_RESOLVER_DEBUG
#include <stdio.h>
#define DEBUG(...) \
	do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
	} while (0)
#else
#define DEBUG(...)  /* don't print messages */
#endif


typedef struct aingle_resolved_writer  aingle_resolved_writer_t;

struct aingle_resolved_writer {
	aingle_value_iface_t  parent;

	/** The reference count for this interface. */
	volatile int  refcount;

	/** The writer schema. */
	aingle_schema_t  wschema;

	/** The reader schema. */
	aingle_schema_t  rschema;

	/* If the reader schema is a union, but the writer schema is
	 * not, this field indicates which branch of the reader union
	 * should be selected. */
	int  reader_union_branch;

	/* The size of the value instances for this resolver. */
	size_t  instance_size;

	/* A function to calculate the instance size once the overall
	 * top-level resolver (and all of its children) have been
	 * constructed. */
	void
	(*calculate_size)(aingle_resolved_writer_t *iface);

	/* A free function for this resolver interface */
	void
	(*free_iface)(aingle_resolved_writer_t *iface, st_table *freeing);

	/* An initialization function for instances of this resolver. */
	int
	(*init)(const aingle_resolved_writer_t *iface, void *self);

	/* A finalization function for instances of this resolver. */
	void
	(*done)(const aingle_resolved_writer_t *iface, void *self);

	/* Clear out any existing wrappers, if any */
	int
	(*reset_wrappers)(const aingle_resolved_writer_t *iface, void *self);
};

#define aingle_resolved_writer_calculate_size(iface) \
	do { \
		if ((iface)->calculate_size != NULL) { \
			(iface)->calculate_size((iface)); \
		} \
	} while (0)
#define aingle_resolved_writer_init(iface, self) \
	((iface)->init == NULL? 0: (iface)->init((iface), (self)))
#define aingle_resolved_writer_done(iface, self) \
	((iface)->done == NULL? (void) 0: (iface)->done((iface), (self)))
#define aingle_resolved_writer_reset_wrappers(iface, self) \
	((iface)->reset_wrappers == NULL? 0: \
	 (iface)->reset_wrappers((iface), (self)))


/*
 * We assume that each instance type in this value contains an an
 * aingle_value_t as its first element, which is the current wrapped
 * value.
 */

void
aingle_resolved_writer_set_dest(aingle_value_t *resolved,
			      aingle_value_t *dest)
{
	aingle_value_t  *self = (aingle_value_t *) resolved->self;
	if (self->self != NULL) {
		aingle_value_decref(self);
	}
	aingle_value_copy_ref(self, dest);
}

void
aingle_resolved_writer_clear_dest(aingle_value_t *resolved)
{
	aingle_value_t  *self = (aingle_value_t *) resolved->self;
	if (self->self != NULL) {
		aingle_value_decref(self);
	}
	self->iface = NULL;
	self->self = NULL;
}

int
aingle_resolved_writer_new_value(aingle_value_iface_t *viface,
			       aingle_value_t *value)
{
	int  rval;
	aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	void  *self = aingle_malloc(iface->instance_size + sizeof(volatile int));
	if (self == NULL) {
		value->iface = NULL;
		value->self = NULL;
		return ENOMEM;
	}

	memset(self, 0, iface->instance_size + sizeof(volatile int));
	volatile int  *refcount = (volatile int *) self;
	self = (char *) self + sizeof(volatile int);

	rval = aingle_resolved_writer_init(iface, self);
	if (rval != 0) {
		aingle_free(self, iface->instance_size + sizeof(volatile int));
		value->iface = NULL;
		value->self = NULL;
		return rval;
	}

	*refcount = 1;
	value->iface = aingle_value_iface_incref(viface);
	value->self = self;
	return 0;
}

static void
aingle_resolved_writer_free_value(const aingle_value_iface_t *viface, void *vself)
{
	aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;

	aingle_resolved_writer_done(iface, vself);
	if (self->self != NULL) {
		aingle_value_decref(self);
	}

	vself = (char *) vself - sizeof(volatile int);
	aingle_free(vself, iface->instance_size + sizeof(volatile int));
}

static void
aingle_resolved_writer_incref(aingle_value_t *value)
{
	/*
	 * This only works if you pass in the top-level value.
	 */

	volatile int  *refcount = (volatile int *) ((char *) value->self - sizeof(volatile int));
	aingle_refcount_inc(refcount);
}

static void
aingle_resolved_writer_decref(aingle_value_t *value)
{
	/*
	 * This only works if you pass in the top-level value.
	 */

	volatile int  *refcount = (volatile int *) ((char *) value->self - sizeof(volatile int));
	if (aingle_refcount_dec(refcount)) {
		aingle_resolved_writer_free_value(value->iface, value->self);
	}
}


static aingle_value_iface_t *
aingle_resolved_writer_incref_iface(aingle_value_iface_t *viface)
{
	aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_refcount_inc(&iface->refcount);
	return viface;
}

static void
free_resolver(aingle_resolved_writer_t *iface, st_table *freeing)
{
	/* First check if we've already started freeing this resolver. */
	if (st_lookup(freeing, (st_data_t) iface, NULL)) {
		DEBUG("Already freed %p", iface);
		return;
	}

	/* Otherwise add this resolver to the freeing set, then free it. */
	st_insert(freeing, (st_data_t) iface, (st_data_t) NULL);
	DEBUG("Freeing resolver %p (%s->%s)", iface,
	      aingle_schema_type_name(iface->wschema),
	      aingle_schema_type_name(iface->rschema));

	iface->free_iface(iface, freeing);
}

static void
aingle_resolved_writer_calculate_size_(aingle_resolved_writer_t *iface)
{
	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_value_t);
}

static void
aingle_resolved_writer_free_iface(aingle_resolved_writer_t *iface, st_table *freeing)
{
	AINGLE_UNUSED(freeing);
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_writer_t, iface);
}

static void
aingle_resolved_writer_decref_iface(aingle_value_iface_t *viface)
{
	aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	DEBUG("Decref resolver %p (before=%d)", iface, iface->refcount);
	if (aingle_refcount_dec(&iface->refcount)) {
		aingle_resolved_writer_t  *iface =
		    container_of(viface, aingle_resolved_writer_t, parent);

		st_table  *freeing = st_init_numtable();
		free_resolver(iface, freeing);
		st_free_table(freeing);
	}
}


static int
aingle_resolved_writer_reset(const aingle_value_iface_t *viface, void *vself)
{
	/*
	 * To reset a wrapped value, we first clear out any wrappers,
	 * and then have the wrapped value reset itself.
	 */

	int  rval;
	aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	check(rval, aingle_resolved_writer_reset_wrappers(iface, vself));
	return aingle_value_reset(self);
}

static aingle_type_t
aingle_resolved_writer_get_type(const aingle_value_iface_t *viface, const void *vself)
{
	AINGLE_UNUSED(vself);
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	return aingle_typeof(iface->wschema);
}

static aingle_schema_t
aingle_resolved_writer_get_schema(const aingle_value_iface_t *viface, const void *vself)
{
	AINGLE_UNUSED(vself);
	aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	return iface->wschema;
}


static aingle_resolved_writer_t *
aingle_resolved_writer_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_writer_t  *self = (aingle_resolved_writer_t *) aingle_new(aingle_resolved_writer_t);
	memset(self, 0, sizeof(aingle_resolved_writer_t));

	self->parent.incref_iface = aingle_resolved_writer_incref_iface;
	self->parent.decref_iface = aingle_resolved_writer_decref_iface;
	self->parent.incref = aingle_resolved_writer_incref;
	self->parent.decref = aingle_resolved_writer_decref;
	self->parent.reset = aingle_resolved_writer_reset;
	self->parent.get_type = aingle_resolved_writer_get_type;
	self->parent.get_schema = aingle_resolved_writer_get_schema;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->reader_union_branch = -1;
	self->calculate_size = aingle_resolved_writer_calculate_size_;
	self->free_iface = aingle_resolved_writer_free_iface;
	self->reset_wrappers = NULL;
	return self;
}

static inline int
aingle_resolved_writer_get_real_dest(const aingle_resolved_writer_t *iface,
				   const aingle_value_t *dest, aingle_value_t *real_dest)
{
	if (iface->reader_union_branch < 0) {
		/*
		 * The reader schema isn't a union, so use the dest
		 * field as-is.
		 */

		*real_dest = *dest;
		return 0;
	}

	DEBUG("Retrieving union branch %d for %s value",
	      iface->reader_union_branch,
	      aingle_schema_type_name(iface->wschema));

	return aingle_value_set_branch(dest, iface->reader_union_branch, real_dest);
}


#define skip_links(schema)					\
	while (is_aingle_link(schema)) {				\
		schema = aingle_schema_link_target(schema);	\
	}


/*-----------------------------------------------------------------------
 * Memoized resolvers
 */

typedef struct aingle_resolved_link_writer  aingle_resolved_link_writer_t;

typedef struct memoize_state_t {
	aingle_memoize_t  mem;
	aingle_resolved_link_writer_t  *links;
} memoize_state_t;

static aingle_resolved_writer_t *
aingle_resolved_writer_new_memoized(memoize_state_t *state,
				  aingle_schema_t wschema, aingle_schema_t rschema);


/*-----------------------------------------------------------------------
 * Reader unions
 */

/*
 * For each AIngle type, we have to check whether the reader schema on its
 * own is compatible, and also whether the reader is a union that
 * contains a compatible type.  The macros in this section help us
 * perform both of these checks with less code.
 */


/**
 * A helper macro that handles the case where neither writer nor reader
 * are unions.  Uses @ref check_func to see if the two schemas are
 * compatible.
 */

#define check_non_union(saved, wschema, rschema, check_func) \
do {								\
	aingle_resolved_writer_t  *self = NULL;			\
	int  rc = check_func(saved, &self, wschema, rschema,	\
			     rschema);				\
	if (self) {						\
		DEBUG("Non-union schemas %s (writer) "		\
		      "and %s (reader) match",			\
		      aingle_schema_type_name(wschema),		\
		      aingle_schema_type_name(rschema));		\
								\
		self->reader_union_branch = -1;			\
		return self;					\
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
	DEBUG("Checking reader union schema");				\
	size_t  num_branches = aingle_schema_union_size(rschema);		\
	unsigned int  i;						\
									\
	for (i = 0; i < num_branches; i++) {				\
		aingle_schema_t  branch_schema =				\
		    aingle_schema_union_branch(rschema, i);		\
		skip_links(branch_schema);				\
									\
		DEBUG("Trying branch %u %s%s%s->%s", i, \
		      is_aingle_link(wschema)? "[": "", \
		      aingle_schema_type_name(wschema), \
		      is_aingle_link(wschema)? "]": "", \
		      aingle_schema_type_name(branch_schema)); \
									\
		aingle_resolved_writer_t  *self = NULL;			\
		int  rc = check_func(saved, &self,			\
				     wschema, branch_schema, rschema);	\
		if (self) {						\
			DEBUG("Reader union branch %d (%s) "		\
			      "and writer %s match",			\
			      i, aingle_schema_type_name(branch_schema),	\
			      aingle_schema_type_name(wschema));		\
			self->reader_union_branch = i;			\
			return self;					\
		} else {						\
			DEBUG("Reader union branch %d (%s) "		\
			      "doesn't match",				\
			      i, aingle_schema_type_name(branch_schema));	\
		}							\
									\
		if (rc) {						\
			return NULL;					\
		}							\
	}								\
									\
	DEBUG("No reader union branches match");			\
} while (0)

/**
 * A helper macro that wraps together check_non_union and
 * check_reader_union for a simple (non-union) writer schema type.
 */

#define check_simple_writer(saved, wschema, rschema, type_name)		\
do {									\
	check_non_union(saved, wschema, rschema, try_##type_name);	\
	check_reader_union(saved, wschema, rschema, try_##type_name);	\
	DEBUG("Writer %s doesn't match reader %s",			\
	      aingle_schema_type_name(wschema),				\
	      aingle_schema_type_name(rschema));				\
	aingle_set_error("Cannot store " #type_name " into %s",		\
		       aingle_schema_type_name(rschema));			\
	return NULL;							\
} while (0)


/*-----------------------------------------------------------------------
 * Recursive schemas
 */

/*
 * Recursive schemas are handled specially; the value implementation for
 * an AINGLE_LINK schema is simply a wrapper around the value
 * implementation for the link's target schema.  The value methods all
 * delegate to the wrapped implementation.
 */

struct aingle_resolved_link_writer {
	aingle_resolved_writer_t  parent;

	/**
	 * A pointer to the “next” link resolver that we've had to
	 * create.  We use this as we're creating the overall top-level
	 * resolver to keep track of which ones we have to fix up
	 * afterwards.
	 */
	aingle_resolved_link_writer_t  *next;

	/** The target's implementation. */
	aingle_resolved_writer_t  *target_resolver;
};

typedef struct aingle_resolved_link_value {
	aingle_value_t  wrapped;
	aingle_value_t  target;
} aingle_resolved_link_value_t;

static void
aingle_resolved_link_writer_calculate_size(aingle_resolved_writer_t *iface)
{
	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for [%s]->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_resolved_link_value_t);
}

static void
aingle_resolved_link_writer_free_iface(aingle_resolved_writer_t *iface, st_table *freeing)
{
	aingle_resolved_link_writer_t  *liface =
	    container_of(iface, aingle_resolved_link_writer_t, parent);
	if (liface->target_resolver != NULL) {
		free_resolver(liface->target_resolver, freeing);
	}
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_link_writer_t, iface);
}

static int
aingle_resolved_link_writer_init(const aingle_resolved_writer_t *iface, void *vself)
{
	int  rval;
	const aingle_resolved_link_writer_t  *liface =
	    container_of(iface, aingle_resolved_link_writer_t, parent);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	size_t  target_instance_size = liface->target_resolver->instance_size;

	self->target.iface = &liface->target_resolver->parent;
	self->target.self = aingle_malloc(target_instance_size);
	if (self->target.self == NULL) {
		return ENOMEM;
	}
	DEBUG("Allocated <%p:%" PRIsz "> for link", self->target.self, target_instance_size);

	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;

	rval = aingle_resolved_writer_init(liface->target_resolver, self->target.self);
	if (rval != 0) {
		aingle_free(self->target.self, target_instance_size);
	}
	return rval;
}

static void
aingle_resolved_link_writer_done(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_link_writer_t  *liface =
	    container_of(iface, aingle_resolved_link_writer_t, parent);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	size_t  target_instance_size = liface->target_resolver->instance_size;
	DEBUG("Freeing <%p:%" PRIsz "> for link", self->target.self, target_instance_size);
	aingle_resolved_writer_done(liface->target_resolver, self->target.self);
	aingle_free(self->target.self, target_instance_size);
	self->target.iface = NULL;
	self->target.self = NULL;
}

static int
aingle_resolved_link_writer_reset(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_link_writer_t  *liface =
	    container_of(iface, aingle_resolved_link_writer_t, parent);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	return aingle_resolved_writer_reset_wrappers
	    (liface->target_resolver, self->target.self);
}

static aingle_type_t
aingle_resolved_link_writer_get_type(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_type(&self->target);
}

static aingle_schema_t
aingle_resolved_link_writer_get_schema(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_schema(&self->target);
}

static int
aingle_resolved_link_writer_get_boolean(const aingle_value_iface_t *iface,
				      const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_boolean(&self->target, out);
}

static int
aingle_resolved_link_writer_get_bytes(const aingle_value_iface_t *iface,
				    const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_bytes(&self->target, buf, size);
}

static int
aingle_resolved_link_writer_grab_bytes(const aingle_value_iface_t *iface,
				     const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_grab_bytes(&self->target, dest);
}

static int
aingle_resolved_link_writer_get_double(const aingle_value_iface_t *iface,
				     const void *vself, double *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_double(&self->target, out);
}

static int
aingle_resolved_link_writer_get_float(const aingle_value_iface_t *iface,
				    const void *vself, float *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_float(&self->target, out);
}

static int
aingle_resolved_link_writer_get_int(const aingle_value_iface_t *iface,
				  const void *vself, int32_t *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_int(&self->target, out);
}

static int
aingle_resolved_link_writer_get_long(const aingle_value_iface_t *iface,
				   const void *vself, int64_t *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_long(&self->target, out);
}

static int
aingle_resolved_link_writer_get_null(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_null(&self->target);
}

static int
aingle_resolved_link_writer_get_string(const aingle_value_iface_t *iface,
				     const void *vself, const char **str, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_string(&self->target, str, size);
}

static int
aingle_resolved_link_writer_grab_string(const aingle_value_iface_t *iface,
				      const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_grab_string(&self->target, dest);
}

static int
aingle_resolved_link_writer_get_enum(const aingle_value_iface_t *iface,
				   const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_enum(&self->target, out);
}

static int
aingle_resolved_link_writer_get_fixed(const aingle_value_iface_t *iface,
				    const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_fixed(&self->target, buf, size);
}

static int
aingle_resolved_link_writer_grab_fixed(const aingle_value_iface_t *iface,
				     const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_grab_fixed(&self->target, dest);
}

static int
aingle_resolved_link_writer_set_boolean(const aingle_value_iface_t *iface,
				      void *vself, int val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_boolean(&self->target, val);
}

static int
aingle_resolved_link_writer_set_bytes(const aingle_value_iface_t *iface,
				    void *vself, void *buf, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_bytes(&self->target, buf, size);
}

static int
aingle_resolved_link_writer_give_bytes(const aingle_value_iface_t *iface,
				     void *vself, aingle_wrapped_buffer_t *buf)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_give_bytes(&self->target, buf);
}

static int
aingle_resolved_link_writer_set_double(const aingle_value_iface_t *iface,
				     void *vself, double val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_double(&self->target, val);
}

static int
aingle_resolved_link_writer_set_float(const aingle_value_iface_t *iface,
				    void *vself, float val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_float(&self->target, val);
}

static int
aingle_resolved_link_writer_set_int(const aingle_value_iface_t *iface,
				  void *vself, int32_t val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_int(&self->target, val);
}

static int
aingle_resolved_link_writer_set_long(const aingle_value_iface_t *iface,
				   void *vself, int64_t val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_long(&self->target, val);
}

static int
aingle_resolved_link_writer_set_null(const aingle_value_iface_t *iface, void *vself)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_null(&self->target);
}

static int
aingle_resolved_link_writer_set_string(const aingle_value_iface_t *iface,
				     void *vself, const char *str)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_string(&self->target, str);
}

static int
aingle_resolved_link_writer_set_string_len(const aingle_value_iface_t *iface,
					 void *vself, const char *str, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_string_len(&self->target, str, size);
}

static int
aingle_resolved_link_writer_give_string_len(const aingle_value_iface_t *iface,
					  void *vself, aingle_wrapped_buffer_t *buf)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_give_string_len(&self->target, buf);
}

static int
aingle_resolved_link_writer_set_enum(const aingle_value_iface_t *iface,
				   void *vself, int val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_enum(&self->target, val);
}

static int
aingle_resolved_link_writer_set_fixed(const aingle_value_iface_t *iface,
				    void *vself, void *buf, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_fixed(&self->target, buf, size);
}

static int
aingle_resolved_link_writer_give_fixed(const aingle_value_iface_t *iface,
				     void *vself, aingle_wrapped_buffer_t *buf)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_give_fixed(&self->target, buf);
}

static int
aingle_resolved_link_writer_get_size(const aingle_value_iface_t *iface,
				   const void *vself, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_size(&self->target, size);
}

static int
aingle_resolved_link_writer_get_by_index(const aingle_value_iface_t *iface,
				       const void *vself, size_t index,
				       aingle_value_t *child, const char **name)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_by_index(&self->target, index, child, name);
}

static int
aingle_resolved_link_writer_get_by_name(const aingle_value_iface_t *iface,
				      const void *vself, const char *name,
				      aingle_value_t *child, size_t *index)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_by_name(&self->target, name, child, index);
}

static int
aingle_resolved_link_writer_get_discriminant(const aingle_value_iface_t *iface,
					   const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_discriminant(&self->target, out);
}

static int
aingle_resolved_link_writer_get_current_branch(const aingle_value_iface_t *iface,
					     const void *vself, aingle_value_t *branch)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_current_branch(&self->target, branch);
}

static int
aingle_resolved_link_writer_append(const aingle_value_iface_t *iface,
				 void *vself, aingle_value_t *child_out,
				 size_t *new_index)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_append(&self->target, child_out, new_index);
}

static int
aingle_resolved_link_writer_add(const aingle_value_iface_t *iface,
			      void *vself, const char *key,
			      aingle_value_t *child, size_t *index, int *is_new)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_add(&self->target, key, child, index, is_new);
}

static int
aingle_resolved_link_writer_set_branch(const aingle_value_iface_t *iface,
				     void *vself, int discriminant,
				     aingle_value_t *branch)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_branch(&self->target, discriminant, branch);
}

static aingle_resolved_link_writer_t *
aingle_resolved_link_writer_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_writer_t  *self = (aingle_resolved_writer_t *) aingle_new(aingle_resolved_link_writer_t);
	memset(self, 0, sizeof(aingle_resolved_link_writer_t));

	self->parent.incref_iface = aingle_resolved_writer_incref_iface;
	self->parent.decref_iface = aingle_resolved_writer_decref_iface;
	self->parent.incref = aingle_resolved_writer_incref;
	self->parent.decref = aingle_resolved_writer_decref;
	self->parent.reset = aingle_resolved_writer_reset;
	self->parent.get_type = aingle_resolved_link_writer_get_type;
	self->parent.get_schema = aingle_resolved_link_writer_get_schema;
	self->parent.get_size = aingle_resolved_link_writer_get_size;
	self->parent.get_by_index = aingle_resolved_link_writer_get_by_index;
	self->parent.get_by_name = aingle_resolved_link_writer_get_by_name;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->reader_union_branch = -1;
	self->calculate_size = aingle_resolved_link_writer_calculate_size;
	self->free_iface = aingle_resolved_link_writer_free_iface;
	self->init = aingle_resolved_link_writer_init;
	self->done = aingle_resolved_link_writer_done;
	self->reset_wrappers = aingle_resolved_link_writer_reset;

	self->parent.get_boolean = aingle_resolved_link_writer_get_boolean;
	self->parent.get_bytes = aingle_resolved_link_writer_get_bytes;
	self->parent.grab_bytes = aingle_resolved_link_writer_grab_bytes;
	self->parent.get_double = aingle_resolved_link_writer_get_double;
	self->parent.get_float = aingle_resolved_link_writer_get_float;
	self->parent.get_int = aingle_resolved_link_writer_get_int;
	self->parent.get_long = aingle_resolved_link_writer_get_long;
	self->parent.get_null = aingle_resolved_link_writer_get_null;
	self->parent.get_string = aingle_resolved_link_writer_get_string;
	self->parent.grab_string = aingle_resolved_link_writer_grab_string;
	self->parent.get_enum = aingle_resolved_link_writer_get_enum;
	self->parent.get_fixed = aingle_resolved_link_writer_get_fixed;
	self->parent.grab_fixed = aingle_resolved_link_writer_grab_fixed;

	self->parent.set_boolean = aingle_resolved_link_writer_set_boolean;
	self->parent.set_bytes = aingle_resolved_link_writer_set_bytes;
	self->parent.give_bytes = aingle_resolved_link_writer_give_bytes;
	self->parent.set_double = aingle_resolved_link_writer_set_double;
	self->parent.set_float = aingle_resolved_link_writer_set_float;
	self->parent.set_int = aingle_resolved_link_writer_set_int;
	self->parent.set_long = aingle_resolved_link_writer_set_long;
	self->parent.set_null = aingle_resolved_link_writer_set_null;
	self->parent.set_string = aingle_resolved_link_writer_set_string;
	self->parent.set_string_len = aingle_resolved_link_writer_set_string_len;
	self->parent.give_string_len = aingle_resolved_link_writer_give_string_len;
	self->parent.set_enum = aingle_resolved_link_writer_set_enum;
	self->parent.set_fixed = aingle_resolved_link_writer_set_fixed;
	self->parent.give_fixed = aingle_resolved_link_writer_give_fixed;

	self->parent.get_size = aingle_resolved_link_writer_get_size;
	self->parent.get_by_index = aingle_resolved_link_writer_get_by_index;
	self->parent.get_by_name = aingle_resolved_link_writer_get_by_name;
	self->parent.get_discriminant = aingle_resolved_link_writer_get_discriminant;
	self->parent.get_current_branch = aingle_resolved_link_writer_get_current_branch;

	self->parent.append = aingle_resolved_link_writer_append;
	self->parent.add = aingle_resolved_link_writer_add;
	self->parent.set_branch = aingle_resolved_link_writer_set_branch;

	return container_of(self, aingle_resolved_link_writer_t, parent);
}

static int
try_link(memoize_state_t *state, aingle_resolved_writer_t **self,
	 aingle_schema_t wschema, aingle_schema_t rschema,
	 aingle_schema_t root_rschema)
{
	AINGLE_UNUSED(rschema);

	/*
	 * For link schemas, we create a special value implementation
	 * that allocates space for its wrapped value at runtime.  This
	 * lets us handle recursive types without having to instantiate
	 * in infinite-size value.
	 */

	aingle_schema_t  wtarget = aingle_schema_link_target(wschema);
	aingle_resolved_link_writer_t  *lself =
	    aingle_resolved_link_writer_create(wtarget, root_rschema);
	aingle_memoize_set(&state->mem, wschema, root_rschema, lself);

	aingle_resolved_writer_t  *target_resolver =
	    aingle_resolved_writer_new_memoized(state, wtarget, rschema);
	if (target_resolver == NULL) {
		aingle_memoize_delete(&state->mem, wschema, root_rschema);
		aingle_value_iface_decref(&lself->parent.parent);
		aingle_prefix_error("Link target isn't compatible: ");
		DEBUG("%s", aingle_strerror());
		return EINVAL;
	}

	lself->target_resolver = target_resolver;
	lself->next = state->links;
	state->links = lself;

	*self = &lself->parent;
	return 0;
}


/*-----------------------------------------------------------------------
 * boolean
 */

static int
aingle_resolved_writer_set_boolean(const aingle_value_iface_t *viface,
				 void *vself, int val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing %s into %p", val? "TRUE": "FALSE", dest.self);
	return aingle_value_set_boolean(&dest, val);
}

static int
try_boolean(memoize_state_t *state, aingle_resolved_writer_t **self,
	    aingle_schema_t wschema, aingle_schema_t rschema,
	    aingle_schema_t root_rschema)
{
	if (is_aingle_boolean(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_boolean = aingle_resolved_writer_set_boolean;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * bytes
 */

static int
aingle_resolved_writer_set_bytes(const aingle_value_iface_t *viface,
			       void *vself, void *buf, size_t size)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing <%p:%" PRIsz "> into %p", buf, size, dest.self);
	return aingle_value_set_bytes(&dest, buf, size);
}

static int
aingle_resolved_writer_give_bytes(const aingle_value_iface_t *viface,
				void *vself, aingle_wrapped_buffer_t *buf)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing [%p] into %p", buf, dest.self);
	return aingle_value_give_bytes(&dest, buf);
}

static int
try_bytes(memoize_state_t *state, aingle_resolved_writer_t **self,
	  aingle_schema_t wschema, aingle_schema_t rschema,
	  aingle_schema_t root_rschema)
{
	if (is_aingle_bytes(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_bytes = aingle_resolved_writer_set_bytes;
		(*self)->parent.give_bytes = aingle_resolved_writer_give_bytes;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * double
 */

static int
aingle_resolved_writer_set_double(const aingle_value_iface_t *viface,
				void *vself, double val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing %le into %p", val, dest.self);
	return aingle_value_set_double(&dest, val);
}

static int
try_double(memoize_state_t *state, aingle_resolved_writer_t **self,
	   aingle_schema_t wschema, aingle_schema_t rschema,
	   aingle_schema_t root_rschema)
{
	if (is_aingle_double(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_double = aingle_resolved_writer_set_double;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * float
 */

static int
aingle_resolved_writer_set_float(const aingle_value_iface_t *viface,
			       void *vself, float val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing %e into %p", val, dest.self);
	return aingle_value_set_float(&dest, val);
}

static int
aingle_resolved_writer_set_float_double(const aingle_value_iface_t *viface,
				      void *vself, float val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Promoting float %e into double %p", val, dest.self);
	return aingle_value_set_double(&dest, val);
}

static int
try_float(memoize_state_t *state, aingle_resolved_writer_t **self,
	  aingle_schema_t wschema, aingle_schema_t rschema,
	  aingle_schema_t root_rschema)
{
	if (is_aingle_float(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_float = aingle_resolved_writer_set_float;
	}

	else if (is_aingle_double(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_float = aingle_resolved_writer_set_float_double;
	}

	return 0;
}


/*-----------------------------------------------------------------------
 * int
 */

static int
aingle_resolved_writer_set_int(const aingle_value_iface_t *viface,
			     void *vself, int32_t val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing %" PRId32 " into %p", val, dest.self);
	return aingle_value_set_int(&dest, val);
}

static int
aingle_resolved_writer_set_int_double(const aingle_value_iface_t *viface,
				    void *vself, int32_t val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Promoting int %" PRId32 " into double %p", val, dest.self);
	return aingle_value_set_double(&dest, val);
}

static int
aingle_resolved_writer_set_int_float(const aingle_value_iface_t *viface,
				   void *vself, int32_t val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Promoting int %" PRId32 " into float %p", val, dest.self);
	return aingle_value_set_float(&dest, (float) val);
}

static int
aingle_resolved_writer_set_int_long(const aingle_value_iface_t *viface,
				  void *vself, int32_t val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Promoting int %" PRId32 " into long %p", val, dest.self);
	return aingle_value_set_long(&dest, val);
}

static int
try_int(memoize_state_t *state, aingle_resolved_writer_t **self,
	aingle_schema_t wschema, aingle_schema_t rschema,
	aingle_schema_t root_rschema)
{
	if (is_aingle_int32(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_int = aingle_resolved_writer_set_int;
	}

	else if (is_aingle_int64(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_int = aingle_resolved_writer_set_int_long;
	}

	else if (is_aingle_double(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_int = aingle_resolved_writer_set_int_double;
	}

	else if (is_aingle_float(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_int = aingle_resolved_writer_set_int_float;
	}

	return 0;
}


/*-----------------------------------------------------------------------
 * long
 */

static int
aingle_resolved_writer_set_long(const aingle_value_iface_t *viface,
			      void *vself, int64_t val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing %" PRId64 " into %p", val, dest.self);
	return aingle_value_set_long(&dest, val);
}

static int
aingle_resolved_writer_set_long_double(const aingle_value_iface_t *viface,
				     void *vself, int64_t val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Promoting long %" PRId64 " into double %p", val, dest.self);
	return aingle_value_set_double(&dest, (double) val);
}

static int
aingle_resolved_writer_set_long_float(const aingle_value_iface_t *viface,
				    void *vself, int64_t val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Promoting long %" PRId64 " into float %p", val, dest.self);
	return aingle_value_set_float(&dest, (float) val);
}

static int
try_long(memoize_state_t *state, aingle_resolved_writer_t **self,
	 aingle_schema_t wschema, aingle_schema_t rschema,
	 aingle_schema_t root_rschema)
{
	if (is_aingle_int64(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_long = aingle_resolved_writer_set_long;
	}

	else if (is_aingle_double(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_long = aingle_resolved_writer_set_long_double;
	}

	else if (is_aingle_float(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_long = aingle_resolved_writer_set_long_float;
	}

	return 0;
}


/*-----------------------------------------------------------------------
 * null
 */

static int
aingle_resolved_writer_set_null(const aingle_value_iface_t *viface,
			      void *vself)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing NULL into %p", dest.self);
	return aingle_value_set_null(&dest);
}

static int
try_null(memoize_state_t *state, aingle_resolved_writer_t **self,
	 aingle_schema_t wschema, aingle_schema_t rschema,
	 aingle_schema_t root_rschema)
{
	if (is_aingle_null(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_null = aingle_resolved_writer_set_null;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * string
 */

static int
aingle_resolved_writer_set_string(const aingle_value_iface_t *viface,
				void *vself, const char *str)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing \"%s\" into %p", str, dest.self);
	return aingle_value_set_string(&dest, str);
}

static int
aingle_resolved_writer_set_string_len(const aingle_value_iface_t *viface,
				    void *vself, const char *str, size_t size)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing <%p:%" PRIsz "> into %p", str, size, dest.self);
	return aingle_value_set_string_len(&dest, str, size);
}

static int
aingle_resolved_writer_give_string_len(const aingle_value_iface_t *viface,
				     void *vself, aingle_wrapped_buffer_t *buf)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing [%p] into %p", buf, dest.self);
	return aingle_value_give_string_len(&dest, buf);
}

static int
try_string(memoize_state_t *state, aingle_resolved_writer_t **self,
	   aingle_schema_t wschema, aingle_schema_t rschema,
	   aingle_schema_t root_rschema)
{
	if (is_aingle_string(rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_string = aingle_resolved_writer_set_string;
		(*self)->parent.set_string_len = aingle_resolved_writer_set_string_len;
		(*self)->parent.give_string_len = aingle_resolved_writer_give_string_len;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * array
 */

typedef struct aingle_resolved_array_writer {
	aingle_resolved_writer_t  parent;
	aingle_resolved_writer_t  *child_resolver;
} aingle_resolved_array_writer_t;

typedef struct aingle_resolved_array_value {
	aingle_value_t  wrapped;
	aingle_raw_array_t  children;
} aingle_resolved_array_value_t;

static void
aingle_resolved_array_writer_calculate_size(aingle_resolved_writer_t *iface)
{
	aingle_resolved_array_writer_t  *aiface =
	    container_of(iface, aingle_resolved_array_writer_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_resolved_array_value_t);

	aingle_resolved_writer_calculate_size(aiface->child_resolver);
}

static void
aingle_resolved_array_writer_free_iface(aingle_resolved_writer_t *iface, st_table *freeing)
{
	aingle_resolved_array_writer_t  *aiface =
	    container_of(iface, aingle_resolved_array_writer_t, parent);
	free_resolver(aiface->child_resolver, freeing);
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_array_writer_t, iface);
}

static int
aingle_resolved_array_writer_init(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_array_writer_t  *aiface =
	    container_of(iface, aingle_resolved_array_writer_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;
	size_t  child_instance_size = aiface->child_resolver->instance_size;
	DEBUG("Initializing child array (child_size=%" PRIsz ")", child_instance_size);
	aingle_raw_array_init(&self->children, child_instance_size);
	return 0;
}

static void
aingle_resolved_array_writer_free_elements(const aingle_resolved_writer_t *child_iface,
					 aingle_resolved_array_value_t *self)
{
	size_t  i;
	for (i = 0; i < aingle_raw_array_size(&self->children); i++) {
		void  *child_self = aingle_raw_array_get_raw(&self->children, i);
		aingle_resolved_writer_done(child_iface, child_self);
	}
}

static void
aingle_resolved_array_writer_done(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_array_writer_t  *aiface =
	    container_of(iface, aingle_resolved_array_writer_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;
	aingle_resolved_array_writer_free_elements(aiface->child_resolver, self);
	aingle_raw_array_done(&self->children);
}

static int
aingle_resolved_array_writer_reset(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_array_writer_t  *aiface =
	    container_of(iface, aingle_resolved_array_writer_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;

	/* Clear out our cache of wrapped children */
	aingle_resolved_array_writer_free_elements(aiface->child_resolver, self);
	aingle_raw_array_clear(&self->children);
	return 0;
}

static int
aingle_resolved_array_writer_get_size(const aingle_value_iface_t *viface,
				    const void *vself, size_t *size)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	const aingle_resolved_array_value_t  *self = (const aingle_resolved_array_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, &self->wrapped, &dest));
	return aingle_value_get_size(&dest, size);
}

static int
aingle_resolved_array_writer_append(const aingle_value_iface_t *viface,
				  void *vself, aingle_value_t *child_out,
				  size_t *new_index)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	const aingle_resolved_array_writer_t  *aiface =
	    container_of(iface, aingle_resolved_array_writer_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, &self->wrapped, &dest));

	child_out->iface = &aiface->child_resolver->parent;
	child_out->self = aingle_raw_array_append(&self->children);
	if (child_out->self == NULL) {
		aingle_set_error("Couldn't expand array");
		return ENOMEM;
	}

	DEBUG("Appending to array %p", dest.self);
	check(rval, aingle_value_append(&dest, (aingle_value_t *) child_out->self, new_index));
	return aingle_resolved_writer_init(aiface->child_resolver, child_out->self);
}

static aingle_resolved_array_writer_t *
aingle_resolved_array_writer_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_writer_t  *self = (aingle_resolved_writer_t *) aingle_new(aingle_resolved_array_writer_t);
	memset(self, 0, sizeof(aingle_resolved_array_writer_t));

	self->parent.incref_iface = aingle_resolved_writer_incref_iface;
	self->parent.decref_iface = aingle_resolved_writer_decref_iface;
	self->parent.incref = aingle_resolved_writer_incref;
	self->parent.decref = aingle_resolved_writer_decref;
	self->parent.reset = aingle_resolved_writer_reset;
	self->parent.get_type = aingle_resolved_writer_get_type;
	self->parent.get_schema = aingle_resolved_writer_get_schema;
	self->parent.get_size = aingle_resolved_array_writer_get_size;
	self->parent.append = aingle_resolved_array_writer_append;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->reader_union_branch = -1;
	self->calculate_size = aingle_resolved_array_writer_calculate_size;
	self->free_iface = aingle_resolved_array_writer_free_iface;
	self->init = aingle_resolved_array_writer_init;
	self->done = aingle_resolved_array_writer_done;
	self->reset_wrappers = aingle_resolved_array_writer_reset;
	return container_of(self, aingle_resolved_array_writer_t, parent);
}

static int
try_array(memoize_state_t *state, aingle_resolved_writer_t **self,
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
	 * compatible themselves.  Try to create an resolver to check
	 * the compatibility.
	 */

	aingle_resolved_array_writer_t  *aself =
	    aingle_resolved_array_writer_create(wschema, root_rschema);
	aingle_memoize_set(&state->mem, wschema, root_rschema, aself);

	aingle_schema_t  witems = aingle_schema_array_items(wschema);
	aingle_schema_t  ritems = aingle_schema_array_items(rschema);

	aingle_resolved_writer_t  *item_resolver =
	    aingle_resolved_writer_new_memoized(state, witems, ritems);
	if (item_resolver == NULL) {
		aingle_memoize_delete(&state->mem, wschema, root_rschema);
		aingle_value_iface_decref(&aself->parent.parent);
		aingle_prefix_error("Array values aren't compatible: ");
		return EINVAL;
	}

	/*
	 * The two schemas are compatible.  Store the item schema's
	 * resolver into the child_resolver field.
	 */

	aself->child_resolver = item_resolver;
	*self = &aself->parent;
	return 0;
}


/*-----------------------------------------------------------------------
 * enum
 */

static int
aingle_resolved_writer_set_enum(const aingle_value_iface_t *viface,
			      void *vself, int val)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing %d into %p", val, dest.self);
	return aingle_value_set_enum(&dest, val);
}

static int
try_enum(memoize_state_t *state, aingle_resolved_writer_t **self,
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

		if (strcmp(wname, rname) == 0) {
			*self = aingle_resolved_writer_create(wschema, root_rschema);
			aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
			(*self)->parent.set_enum = aingle_resolved_writer_set_enum;
		}
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * fixed
 */

static int
aingle_resolved_writer_set_fixed(const aingle_value_iface_t *viface,
			       void *vself, void *buf, size_t size)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing <%p:%" PRIsz "> into (fixed) %p", buf, size, dest.self);
	return aingle_value_set_fixed(&dest, buf, size);
}

static int
aingle_resolved_writer_give_fixed(const aingle_value_iface_t *viface,
				void *vself, aingle_wrapped_buffer_t *buf)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, self, &dest));
	DEBUG("Storing [%p] into (fixed) %p", buf, dest.self);
	return aingle_value_give_fixed(&dest, buf);
}

static int
try_fixed(memoize_state_t *state, aingle_resolved_writer_t **self,
	  aingle_schema_t wschema, aingle_schema_t rschema,
	  aingle_schema_t root_rschema)
{
	/*
	 * Fixed schemas need the same name and size to be compatible.
	 */

	if (aingle_schema_equal(wschema, rschema)) {
		*self = aingle_resolved_writer_create(wschema, root_rschema);
		aingle_memoize_set(&state->mem, wschema, root_rschema, *self);
		(*self)->parent.set_fixed = aingle_resolved_writer_set_fixed;
		(*self)->parent.give_fixed = aingle_resolved_writer_give_fixed;
	}
	return 0;
}


/*-----------------------------------------------------------------------
 * map
 */

typedef struct aingle_resolved_map_writer {
	aingle_resolved_writer_t  parent;
	aingle_resolved_writer_t  *child_resolver;
} aingle_resolved_map_writer_t;

typedef struct aingle_resolved_map_value {
	aingle_value_t  wrapped;
	aingle_raw_array_t  children;
} aingle_resolved_map_value_t;

static void
aingle_resolved_map_writer_calculate_size(aingle_resolved_writer_t *iface)
{
	aingle_resolved_map_writer_t  *miface =
	    container_of(iface, aingle_resolved_map_writer_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_resolved_map_value_t);

	aingle_resolved_writer_calculate_size(miface->child_resolver);
}

static void
aingle_resolved_map_writer_free_iface(aingle_resolved_writer_t *iface, st_table *freeing)
{
	aingle_resolved_map_writer_t  *miface =
	    container_of(iface, aingle_resolved_map_writer_t, parent);
	free_resolver(miface->child_resolver, freeing);
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_map_writer_t, iface);
}

static int
aingle_resolved_map_writer_init(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_map_writer_t  *miface =
	    container_of(iface, aingle_resolved_map_writer_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;
	size_t  child_instance_size = miface->child_resolver->instance_size;
	DEBUG("Initializing child array for map (child_size=%" PRIsz ")", child_instance_size);
	aingle_raw_array_init(&self->children, child_instance_size);
	return 0;
}

static void
aingle_resolved_map_writer_free_elements(const aingle_resolved_writer_t *child_iface,
				       aingle_resolved_map_value_t *self)
{
	size_t  i;
	for (i = 0; i < aingle_raw_array_size(&self->children); i++) {
		void  *child_self = aingle_raw_array_get_raw(&self->children, i);
		aingle_resolved_writer_done(child_iface, child_self);
	}
}

static void
aingle_resolved_map_writer_done(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_map_writer_t  *miface =
	    container_of(iface, aingle_resolved_map_writer_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;
	aingle_resolved_map_writer_free_elements(miface->child_resolver, self);
	aingle_raw_array_done(&self->children);
}

static int
aingle_resolved_map_writer_reset(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_map_writer_t  *miface =
	    container_of(iface, aingle_resolved_map_writer_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;

	/* Clear out our cache of wrapped children */
	aingle_resolved_map_writer_free_elements(miface->child_resolver, self);
	return 0;
}

static int
aingle_resolved_map_writer_get_size(const aingle_value_iface_t *viface,
				  const void *vself, size_t *size)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	const aingle_resolved_map_value_t  *self = (const aingle_resolved_map_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, &self->wrapped, &dest));
	return aingle_value_get_size(&dest, size);
}

static int
aingle_resolved_map_writer_add(const aingle_value_iface_t *viface,
			     void *vself, const char *key,
			     aingle_value_t *child, size_t *index, int *is_new)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	const aingle_resolved_map_writer_t  *miface =
	    container_of(iface, aingle_resolved_map_writer_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;
	aingle_value_t  dest;
	check(rval, aingle_resolved_writer_get_real_dest(iface, &self->wrapped, &dest));

	/*
	 * This is a bit convoluted.  We need to stash the wrapped child
	 * value somewhere in our children array.  But we don't know
	 * where to put it until the wrapped map tells us whether this
	 * is a new value, and if not, which index the value should go
	 * in.
	 */

	aingle_value_t  real_child;
	size_t  real_index;
	int  real_is_new;

	DEBUG("Adding %s to map %p", key, dest.self);
	check(rval, aingle_value_add(&dest, key, &real_child, &real_index, &real_is_new));

	child->iface = &miface->child_resolver->parent;
	if (real_is_new) {
		child->self = aingle_raw_array_append(&self->children);
		DEBUG("Element is new (child resolver=%p)", child->self);
		if (child->self == NULL) {
			aingle_set_error("Couldn't expand map");
			return ENOMEM;
		}
		check(rval, aingle_resolved_writer_init
		      (miface->child_resolver, child->self));
	} else {
		child->self = aingle_raw_array_get_raw(&self->children, real_index);
		DEBUG("Element is old (child resolver=%p)", child->self);
	}
	aingle_value_t  *child_vself = (aingle_value_t *) child->self;
	*child_vself = real_child;

	if (index != NULL) {
		*index = real_index;
	}
	if (is_new != NULL) {
		*is_new = real_is_new;
	}
	return 0;
}

static aingle_resolved_map_writer_t *
aingle_resolved_map_writer_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_writer_t  *self = (aingle_resolved_writer_t *) aingle_new(aingle_resolved_map_writer_t);
	memset(self, 0, sizeof(aingle_resolved_map_writer_t));

	self->parent.incref_iface = aingle_resolved_writer_incref_iface;
	self->parent.decref_iface = aingle_resolved_writer_decref_iface;
	self->parent.incref = aingle_resolved_writer_incref;
	self->parent.decref = aingle_resolved_writer_decref;
	self->parent.reset = aingle_resolved_writer_reset;
	self->parent.get_type = aingle_resolved_writer_get_type;
	self->parent.get_schema = aingle_resolved_writer_get_schema;
	self->parent.get_size = aingle_resolved_map_writer_get_size;
	self->parent.add = aingle_resolved_map_writer_add;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->reader_union_branch = -1;
	self->calculate_size = aingle_resolved_map_writer_calculate_size;
	self->free_iface = aingle_resolved_map_writer_free_iface;
	self->init = aingle_resolved_map_writer_init;
	self->done = aingle_resolved_map_writer_done;
	self->reset_wrappers = aingle_resolved_map_writer_reset;
	return container_of(self, aingle_resolved_map_writer_t, parent);
}

static int
try_map(memoize_state_t *state, aingle_resolved_writer_t **self,
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
	 * Map schemas have to have compatible element schemas to be
	 * compatible themselves.  Try to create an resolver to check
	 * the compatibility.
	 */

	aingle_resolved_map_writer_t  *mself =
	    aingle_resolved_map_writer_create(wschema, root_rschema);
	aingle_memoize_set(&state->mem, wschema, root_rschema, mself);

	aingle_schema_t  witems = aingle_schema_map_values(wschema);
	aingle_schema_t  ritems = aingle_schema_map_values(rschema);

	aingle_resolved_writer_t  *item_resolver =
	    aingle_resolved_writer_new_memoized(state, witems, ritems);
	if (item_resolver == NULL) {
		aingle_memoize_delete(&state->mem, wschema, root_rschema);
		aingle_value_iface_decref(&mself->parent.parent);
		aingle_prefix_error("Map values aren't compatible: ");
		return EINVAL;
	}

	/*
	 * The two schemas are compatible.  Store the item schema's
	 * resolver into the child_resolver field.
	 */

	mself->child_resolver = item_resolver;
	*self = &mself->parent;
	return 0;
}


/*-----------------------------------------------------------------------
 * record
 */

typedef struct aingle_resolved_record_writer {
	aingle_resolved_writer_t  parent;
	size_t  field_count;
	size_t  *field_offsets;
	aingle_resolved_writer_t  **field_resolvers;
	size_t  *index_mapping;
} aingle_resolved_record_writer_t;

typedef struct aingle_resolved_record_value {
	aingle_value_t  wrapped;
	/* The rest of the struct is taken up by the inline storage
	 * needed for each field. */
} aingle_resolved_record_value_t;

/** Return a pointer to the given field within a record struct. */
#define aingle_resolved_record_field(iface, rec, index) \
	(((char *) (rec)) + (iface)->field_offsets[(index)])


static void
aingle_resolved_record_writer_calculate_size(aingle_resolved_writer_t *iface)
{
	aingle_resolved_record_writer_t  *riface =
	    container_of(iface, aingle_resolved_record_writer_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));

	/*
	 * Once we've figured out which writer fields we actually need,
	 * calculate an offset for each one.
	 */

	size_t  wi;
	size_t  next_offset = sizeof(aingle_resolved_record_value_t);
	for (wi = 0; wi < riface->field_count; wi++) {
		riface->field_offsets[wi] = next_offset;
		if (riface->field_resolvers[wi] != NULL) {
			aingle_resolved_writer_calculate_size
			    (riface->field_resolvers[wi]);
			size_t  field_size =
			    riface->field_resolvers[wi]->instance_size;
			DEBUG("Field %" PRIsz " has size %" PRIsz, wi, field_size);
			next_offset += field_size;
		} else {
			DEBUG("Field %" PRIsz " is being skipped", wi);
		}
	}

	DEBUG("Record has size %" PRIsz, next_offset);
	iface->instance_size = next_offset;
}

static void
aingle_resolved_record_writer_free_iface(aingle_resolved_writer_t *iface, st_table *freeing)
{
	aingle_resolved_record_writer_t  *riface =
	    container_of(iface, aingle_resolved_record_writer_t, parent);

	if (riface->field_offsets != NULL) {
		aingle_free(riface->field_offsets,
			  riface->field_count * sizeof(size_t));
	}

	if (riface->field_resolvers != NULL) {
		size_t  i;
		for (i = 0; i < riface->field_count; i++) {
			if (riface->field_resolvers[i] != NULL) {
				DEBUG("Freeing field %" PRIsz " %p", i,
				      riface->field_resolvers[i]);
				free_resolver(riface->field_resolvers[i], freeing);
			}
		}
		aingle_free(riface->field_resolvers,
			  riface->field_count * sizeof(aingle_resolved_writer_t *));
	}

	if (riface->index_mapping != NULL) {
		aingle_free(riface->index_mapping,
			  riface->field_count * sizeof(size_t));
	}

	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_record_writer_t, iface);
}

static int
aingle_resolved_record_writer_init(const aingle_resolved_writer_t *iface, void *vself)
{
	int  rval;
	const aingle_resolved_record_writer_t  *riface =
	    container_of(iface, aingle_resolved_record_writer_t, parent);
	aingle_resolved_record_value_t  *self = (aingle_resolved_record_value_t *) vself;

	/* Initialize each field */
	size_t  i;
	for (i = 0; i < riface->field_count; i++) {
		if (riface->field_resolvers[i] != NULL) {
			check(rval, aingle_resolved_writer_init
			      (riface->field_resolvers[i],
			       aingle_resolved_record_field(riface, self, i)));
		}
	}

	return 0;
}

static void
aingle_resolved_record_writer_done(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_record_writer_t  *riface =
	    container_of(iface, aingle_resolved_record_writer_t, parent);
	aingle_resolved_record_value_t  *self = (aingle_resolved_record_value_t *) vself;

	/* Finalize each field */
	size_t  i;
	for (i = 0; i < riface->field_count; i++) {
		if (riface->field_resolvers[i] != NULL) {
			aingle_resolved_writer_done
			    (riface->field_resolvers[i],
			     aingle_resolved_record_field(riface, self, i));
		}
	}
}

static int
aingle_resolved_record_writer_reset(const aingle_resolved_writer_t *iface, void *vself)
{
	int  rval;
	const aingle_resolved_record_writer_t  *riface =
	    container_of(iface, aingle_resolved_record_writer_t, parent);
	aingle_resolved_record_value_t  *self = (aingle_resolved_record_value_t *) vself;

	/* Reset each field */
	size_t  i;
	for (i = 0; i < riface->field_count; i++) {
		if (riface->field_resolvers[i] != NULL) {
			check(rval, aingle_resolved_writer_reset_wrappers
			      (riface->field_resolvers[i],
			       aingle_resolved_record_field(riface, self, i)));
		}
	}

	return 0;
}

static int
aingle_resolved_record_writer_get_size(const aingle_value_iface_t *viface,
				     const void *vself, size_t *size)
{
	AINGLE_UNUSED(vself);
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	const aingle_resolved_record_writer_t  *riface =
	    container_of(iface, aingle_resolved_record_writer_t, parent);
	*size = riface->field_count;
	return 0;
}

static int
aingle_resolved_record_writer_get_by_index(const aingle_value_iface_t *viface,
					 const void *vself, size_t index,
					 aingle_value_t *child, const char **name)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	const aingle_resolved_record_writer_t  *riface =
	    container_of(iface, aingle_resolved_record_writer_t, parent);
	const aingle_resolved_record_value_t  *self = (const aingle_resolved_record_value_t *) vself;
	aingle_value_t  dest;

	DEBUG("Getting writer field %" PRIsz " from record %p", index, self);
	if (riface->field_resolvers[index] == NULL) {
		DEBUG("Reader doesn't have field, skipping");
		child->iface = NULL;
		child->self = NULL;
		return 0;
	}

	check(rval, aingle_resolved_writer_get_real_dest(iface, &self->wrapped, &dest));
	size_t  reader_index = riface->index_mapping[index];
	DEBUG("  Reader field is %" PRIsz, reader_index);
	child->iface = &riface->field_resolvers[index]->parent;
	child->self = aingle_resolved_record_field(riface, self, index);

	return aingle_value_get_by_index(&dest, reader_index, (aingle_value_t *) child->self, name);
}

static int
aingle_resolved_record_writer_get_by_name(const aingle_value_iface_t *viface,
					const void *vself, const char *name,
					aingle_value_t *child, size_t *index)
{
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);

	int  wi = aingle_schema_record_field_get_index(iface->wschema, name);
	if (wi == -1) {
		aingle_set_error("Record doesn't have field named %s", name);
		return EINVAL;
	}

	DEBUG("Writer field %s is at index %d", name, wi);
	if (index != NULL) {
		*index = wi;
	}
	return aingle_resolved_record_writer_get_by_index(viface, vself, wi, child, NULL);
}

static aingle_resolved_record_writer_t *
aingle_resolved_record_writer_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_writer_t  *self = (aingle_resolved_writer_t *) aingle_new(aingle_resolved_record_writer_t);
	memset(self, 0, sizeof(aingle_resolved_record_writer_t));

	self->parent.incref_iface = aingle_resolved_writer_incref_iface;
	self->parent.decref_iface = aingle_resolved_writer_decref_iface;
	self->parent.incref = aingle_resolved_writer_incref;
	self->parent.decref = aingle_resolved_writer_decref;
	self->parent.reset = aingle_resolved_writer_reset;
	self->parent.get_type = aingle_resolved_writer_get_type;
	self->parent.get_schema = aingle_resolved_writer_get_schema;
	self->parent.get_size = aingle_resolved_record_writer_get_size;
	self->parent.get_by_index = aingle_resolved_record_writer_get_by_index;
	self->parent.get_by_name = aingle_resolved_record_writer_get_by_name;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->reader_union_branch = -1;
	self->calculate_size = aingle_resolved_record_writer_calculate_size;
	self->free_iface = aingle_resolved_record_writer_free_iface;
	self->init = aingle_resolved_record_writer_init;
	self->done = aingle_resolved_record_writer_done;
	self->reset_wrappers = aingle_resolved_record_writer_reset;
	return container_of(self, aingle_resolved_record_writer_t, parent);
}

static int
try_record(memoize_state_t *state, aingle_resolved_writer_t **self,
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

	if (strcmp(wname, rname) != 0) {
		return 0;
	}

	/*
	 * Categorize the fields in the record schemas.  Fields that are
	 * only in the writer are ignored.  Fields that are only in the
	 * reader raise a schema mismatch error, unless the field has a
	 * default value.  Fields that are in both are resolved
	 * recursively.
	 *
	 * The field_resolvers array will contain an aingle_value_iface_t
	 * for each field in the writer schema.  To build this array, we
	 * loop through the fields of the reader schema.  If that field
	 * is also in the writer schema, we resolve them recursively,
	 * and store the resolver into the array.  If the field isn't in
	 * the writer schema, we raise an error.  (TODO: Eventually,
	 * we'll handle default values here.)  After this loop finishes,
	 * any NULLs in the field_resolvers array will represent fields
	 * in the writer but not the reader; these fields will be
	 * skipped when processing the input.
	 */

	aingle_resolved_record_writer_t  *rself =
	    aingle_resolved_record_writer_create(wschema, root_rschema);
	aingle_memoize_set(&state->mem, wschema, root_rschema, rself);

	size_t  wfields = aingle_schema_record_size(wschema);
	size_t  rfields = aingle_schema_record_size(rschema);

	DEBUG("Checking writer record schema %s", wname);

	aingle_resolved_writer_t  **field_resolvers =
	    (aingle_resolved_writer_t **) aingle_calloc(wfields, sizeof(aingle_resolved_writer_t *));
	size_t  *field_offsets = (size_t *) aingle_calloc(wfields, sizeof(size_t));
	size_t  *index_mapping = (size_t *) aingle_calloc(wfields, sizeof(size_t));

	size_t  ri;
	for (ri = 0; ri < rfields; ri++) {
		aingle_schema_t  rfield =
		    aingle_schema_record_field_get_by_index(rschema, ri);
		const char  *field_name =
		    aingle_schema_record_field_name(rschema, ri);

		DEBUG("Resolving reader record field %" PRIsz " (%s)", ri, field_name);

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

			DEBUG("Field %s isn't in writer", field_name);

			/* Allow missing fields in the writer. They
			 * will default to zero. So skip over the
			 * missing field, and continue building the
			 * resolver. Note also that all missing values
			 * are zero because aingle_generic_value_new()
			 * initializes all values of the reader to 0
			 * on creation. This is a work-around because
			 * default values are not implemented yet.
			 */
			#ifdef AINGLE_ALLOW_MISSING_FIELDS_IN_RESOLVED_WRITER
			continue;
			#else
			aingle_set_error("Reader field %s doesn't appear in writer",
				       field_name);
			goto error;
			#endif
		}

		/*
		 * Try to recursively resolve the schemas for this
		 * field.  If they're not compatible, that's an error.
		 */

		aingle_schema_t  wfield =
		    aingle_schema_record_field_get_by_index(wschema, wi);
		aingle_resolved_writer_t  *field_resolver =
		    aingle_resolved_writer_new_memoized(state, wfield, rfield);

		if (field_resolver == NULL) {
			aingle_prefix_error("Field %s isn't compatible: ", field_name);
			goto error;
		}

		/*
		 * Save the details for this field.
		 */

		DEBUG("Found match for field %s (%" PRIsz " in reader, %d in writer)",
		      field_name, ri, wi);
		field_resolvers[wi] = field_resolver;
		index_mapping[wi] = ri;
	}

	/*
	 * We might not have found matches for all of the writer fields,
	 * but that's okay — any extras will be ignored.
	 */

	rself->field_count = wfields;
	rself->field_offsets = field_offsets;
	rself->field_resolvers = field_resolvers;
	rself->index_mapping = index_mapping;
	*self = &rself->parent;
	return 0;

error:
	/*
	 * Clean up any resolver we might have already created.
	 */

	aingle_memoize_delete(&state->mem, wschema, root_rschema);
	aingle_value_iface_decref(&rself->parent.parent);

	{
		unsigned int  i;
		for (i = 0; i < wfields; i++) {
			if (field_resolvers[i]) {
				aingle_value_iface_decref(&field_resolvers[i]->parent);
			}
		}
	}

	aingle_free(field_resolvers, wfields * sizeof(aingle_resolved_writer_t *));
	aingle_free(field_offsets, wfields * sizeof(size_t));
	aingle_free(index_mapping, wfields * sizeof(size_t));
	return EINVAL;
}


/*-----------------------------------------------------------------------
 * union
 */

typedef struct aingle_resolved_union_writer {
	aingle_resolved_writer_t  parent;
	size_t  branch_count;
	aingle_resolved_writer_t  **branch_resolvers;
} aingle_resolved_union_writer_t;

typedef struct aingle_resolved_union_value {
	aingle_value_t  wrapped;

	/** The currently active branch of the union.  -1 if no branch
	 * is selected. */
	int  discriminant;

	/* The rest of the struct is taken up by the inline storage
	 * needed for the active branch. */
} aingle_resolved_union_value_t;

/** Return a pointer to the active branch within a union struct. */
#define aingle_resolved_union_branch(_union) \
	(((char *) (_union)) + sizeof(aingle_resolved_union_value_t))


static void
aingle_resolved_union_writer_calculate_size(aingle_resolved_writer_t *iface)
{
	aingle_resolved_union_writer_t  *uiface =
	    container_of(iface, aingle_resolved_union_writer_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));

	size_t  i;
	size_t  max_branch_size = 0;
	for (i = 0; i < uiface->branch_count; i++) {
		if (uiface->branch_resolvers[i] == NULL) {
			DEBUG("No match for writer union branch %" PRIsz, i);
		} else {
			aingle_resolved_writer_calculate_size
			    (uiface->branch_resolvers[i]);
			size_t  branch_size =
			    uiface->branch_resolvers[i]->instance_size;
			DEBUG("Writer branch %" PRIsz " has size %" PRIsz, i, branch_size);
			if (branch_size > max_branch_size) {
				max_branch_size = branch_size;
			}
		}
	}

	DEBUG("Maximum branch size is %" PRIsz, max_branch_size);
	iface->instance_size =
	    sizeof(aingle_resolved_union_value_t) + max_branch_size;
	DEBUG("Total union size is %" PRIsz, iface->instance_size);
}

static void
aingle_resolved_union_writer_free_iface(aingle_resolved_writer_t *iface, st_table *freeing)
{
	aingle_resolved_union_writer_t  *uiface =
	    container_of(iface, aingle_resolved_union_writer_t, parent);

	if (uiface->branch_resolvers != NULL) {
		size_t  i;
		for (i = 0; i < uiface->branch_count; i++) {
			if (uiface->branch_resolvers[i] != NULL) {
				free_resolver(uiface->branch_resolvers[i], freeing);
			}
		}
		aingle_free(uiface->branch_resolvers,
			  uiface->branch_count * sizeof(aingle_resolved_writer_t *));
	}

	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_union_writer_t, iface);
}

static int
aingle_resolved_union_writer_init(const aingle_resolved_writer_t *iface, void *vself)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_union_value_t  *self = (aingle_resolved_union_value_t *) vself;
	self->discriminant = -1;
	return 0;
}

static void
aingle_resolved_union_writer_done(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_union_writer_t  *uiface =
	    container_of(iface, aingle_resolved_union_writer_t, parent);
	aingle_resolved_union_value_t  *self = (aingle_resolved_union_value_t *) vself;
	if (self->discriminant >= 0) {
		aingle_resolved_writer_done
		    (uiface->branch_resolvers[self->discriminant],
		     aingle_resolved_union_branch(self));
		self->discriminant = -1;
	}
}

static int
aingle_resolved_union_writer_reset(const aingle_resolved_writer_t *iface, void *vself)
{
	const aingle_resolved_union_writer_t  *uiface =
	    container_of(iface, aingle_resolved_union_writer_t, parent);
	aingle_resolved_union_value_t  *self = (aingle_resolved_union_value_t *) vself;

	/* Keep the same branch selected, for the common case that we're
	 * about to reuse it. */
	if (self->discriminant >= 0) {
		return aingle_resolved_writer_reset_wrappers
		    (uiface->branch_resolvers[self->discriminant],
		     aingle_resolved_union_branch(self));
	}

	return 0;
}

static int
aingle_resolved_union_writer_set_branch(const aingle_value_iface_t *viface,
				      void *vself, int discriminant,
				      aingle_value_t *branch)
{
	int  rval;
	const aingle_resolved_writer_t  *iface =
	    container_of(viface, aingle_resolved_writer_t, parent);
	const aingle_resolved_union_writer_t  *uiface =
	    container_of(iface, aingle_resolved_union_writer_t, parent);
	aingle_resolved_union_value_t  *self = (aingle_resolved_union_value_t *) vself;

	DEBUG("Getting writer branch %d from union %p", discriminant, vself);
	aingle_resolved_writer_t  *branch_resolver =
	    uiface->branch_resolvers[discriminant];
	if (branch_resolver == NULL) {
		DEBUG("Reader doesn't have branch, skipping");
		aingle_set_error("Writer union branch %d is incompatible "
			       "with reader schema \"%s\"",
			       discriminant, aingle_schema_type_name(iface->rschema));
		return EINVAL;
	}

	if (self->discriminant == discriminant) {
		DEBUG("Writer branch %d already selected", discriminant);
	} else {
		if (self->discriminant >= 0) {
			DEBUG("Finalizing old writer branch %d", self->discriminant);
			aingle_resolved_writer_done
			    (uiface->branch_resolvers[self->discriminant],
			     aingle_resolved_union_branch(self));
		}
		DEBUG("Initializing writer branch %d", discriminant);
		check(rval, aingle_resolved_writer_init
		      (uiface->branch_resolvers[discriminant],
		       aingle_resolved_union_branch(self)));
		self->discriminant = discriminant;
	}

	branch->iface = &branch_resolver->parent;
	branch->self = aingle_resolved_union_branch(self);
	aingle_value_t  *branch_vself = (aingle_value_t *) branch->self;
	*branch_vself = self->wrapped;
	return 0;
}

static aingle_resolved_union_writer_t *
aingle_resolved_union_writer_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_writer_t  *self = (aingle_resolved_writer_t *) aingle_new(aingle_resolved_union_writer_t);
	memset(self, 0, sizeof(aingle_resolved_union_writer_t));

	self->parent.incref_iface = aingle_resolved_writer_incref_iface;
	self->parent.decref_iface = aingle_resolved_writer_decref_iface;
	self->parent.incref = aingle_resolved_writer_incref;
	self->parent.decref = aingle_resolved_writer_decref;
	self->parent.reset = aingle_resolved_writer_reset;
	self->parent.get_type = aingle_resolved_writer_get_type;
	self->parent.get_schema = aingle_resolved_writer_get_schema;
	self->parent.set_branch = aingle_resolved_union_writer_set_branch;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->reader_union_branch = -1;
	self->calculate_size = aingle_resolved_union_writer_calculate_size;
	self->free_iface = aingle_resolved_union_writer_free_iface;
	self->init = aingle_resolved_union_writer_init;
	self->done = aingle_resolved_union_writer_done;
	self->reset_wrappers = aingle_resolved_union_writer_reset;
	return container_of(self, aingle_resolved_union_writer_t, parent);
}

static aingle_resolved_writer_t *
try_union(memoize_state_t *state,
	  aingle_schema_t wschema, aingle_schema_t rschema)
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
	 * branch, we stash away the recursive resolver into the
	 * branch_resolvers array.  A NULL entry in this array means
	 * that that branch isn't compatible with the reader.  This
	 * isn't an immediate schema resolution error, since we allow
	 * incompatible branches in the types as long as that branch
	 * never appears in the actual data.  We only return an error if
	 * there are *no* branches that are compatible.
	 */

	size_t  branch_count = aingle_schema_union_size(wschema);
	DEBUG("Checking %" PRIsz "-branch writer union schema", branch_count);

	aingle_resolved_union_writer_t  *uself =
	    aingle_resolved_union_writer_create(wschema, rschema);
	aingle_memoize_set(&state->mem, wschema, rschema, uself);

	aingle_resolved_writer_t  **branch_resolvers =
	    (aingle_resolved_writer_t **) aingle_calloc(branch_count, sizeof(aingle_resolved_writer_t *));
	int  some_branch_compatible = 0;

	size_t  i;
	for (i = 0; i < branch_count; i++) {
		aingle_schema_t  branch_schema =
		    aingle_schema_union_branch(wschema, i);

		DEBUG("Resolving writer union branch %" PRIsz " (%s)", i,
		      aingle_schema_type_name(branch_schema));

		/*
		 * Try to recursively resolve this branch of the writer
		 * union.  Don't raise an error if this fails — it's
		 * okay for some of the branches to not be compatible
		 * with the reader, as long as those branches never
		 * appear in the input.
		 */

		branch_resolvers[i] =
		    aingle_resolved_writer_new_memoized(state, branch_schema, rschema);
		if (branch_resolvers[i] == NULL) {
			DEBUG("No match for writer union branch %" PRIsz, i);
		} else {
			DEBUG("Found match for writer union branch %" PRIsz, i);
			some_branch_compatible = 1;
		}
	}

	/*
	 * As long as there's at least one branch that's compatible with
	 * the reader, then we consider this schema resolution a
	 * success.
	 */

	if (!some_branch_compatible) {
		DEBUG("No writer union branches match");
		aingle_set_error("No branches in the writer are compatible "
			       "with reader schema %s",
			       aingle_schema_type_name(rschema));
		goto error;
	}

	uself->branch_count = branch_count;
	uself->branch_resolvers = branch_resolvers;
	return &uself->parent;

error:
	/*
	 * Clean up any resolver we might have already created.
	 */

	aingle_memoize_delete(&state->mem, wschema, rschema);
	aingle_value_iface_decref(&uself->parent.parent);

	{
		unsigned int  i;
		for (i = 0; i < branch_count; i++) {
			if (branch_resolvers[i]) {
				aingle_value_iface_decref(&branch_resolvers[i]->parent);
			}
		}
	}

	aingle_free(branch_resolvers, branch_count * sizeof(aingle_resolved_writer_t *));
	return NULL;
}


/*-----------------------------------------------------------------------
 * Schema type dispatcher
 */

static aingle_resolved_writer_t *
aingle_resolved_writer_new_memoized(memoize_state_t *state,
				  aingle_schema_t wschema, aingle_schema_t rschema)
{
	check_param(NULL, is_aingle_schema(wschema), "writer schema");
	check_param(NULL, is_aingle_schema(rschema), "reader schema");

	skip_links(rschema);

	/*
	 * First see if we've already matched these two schemas.  If so,
	 * just return that resolver.
	 */

	aingle_resolved_writer_t  *saved = NULL;
	if (aingle_memoize_get(&state->mem, wschema, rschema, (void **) &saved)) {
		DEBUG("Already resolved %s%s%s->%s",
		      is_aingle_link(wschema)? "[": "",
		      aingle_schema_type_name(wschema),
		      is_aingle_link(wschema)? "]": "",
		      aingle_schema_type_name(rschema));
		aingle_value_iface_incref(&saved->parent);
		return saved;
	} else {
		DEBUG("Resolving %s%s%s->%s",
		      is_aingle_link(wschema)? "[": "",
		      aingle_schema_type_name(wschema),
		      is_aingle_link(wschema)? "]": "",
		      aingle_schema_type_name(rschema));
	}

	/*
	 * Otherwise we have some work to do.
	 */

	switch (aingle_typeof(wschema))
	{
		case AINGLE_BOOLEAN:
			check_simple_writer(state, wschema, rschema, boolean);
			return NULL;

		case AINGLE_BYTES:
			check_simple_writer(state, wschema, rschema, bytes);
			return NULL;

		case AINGLE_DOUBLE:
			check_simple_writer(state, wschema, rschema, double);
			return NULL;

		case AINGLE_FLOAT:
			check_simple_writer(state, wschema, rschema, float);
			return NULL;

		case AINGLE_INT32:
			check_simple_writer(state, wschema, rschema, int);
			return NULL;

		case AINGLE_INT64:
			check_simple_writer(state, wschema, rschema, long);
			return NULL;

		case AINGLE_NULL:
			check_simple_writer(state, wschema, rschema, null);
			return NULL;

		case AINGLE_STRING:
			check_simple_writer(state, wschema, rschema, string);
			return NULL;

		case AINGLE_ARRAY:
			check_simple_writer(state, wschema, rschema, array);
			return NULL;

		case AINGLE_ENUM:
			check_simple_writer(state, wschema, rschema, enum);
			return NULL;

		case AINGLE_FIXED:
			check_simple_writer(state, wschema, rschema, fixed);
			return NULL;

		case AINGLE_MAP:
			check_simple_writer(state, wschema, rschema, map);
			return NULL;

		case AINGLE_RECORD:
			check_simple_writer(state, wschema, rschema, record);
			return NULL;

		case AINGLE_UNION:
			return try_union(state, wschema, rschema);

		case AINGLE_LINK:
			check_simple_writer(state, wschema, rschema, link);
			return NULL;

		default:
			aingle_set_error("Unknown schema type");
			return NULL;
	}

	return NULL;
}


aingle_value_iface_t *
aingle_resolved_writer_new(aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * Create a state to keep track of the value implementations
	 * that we create for each subschema.
	 */

	memoize_state_t  state;
	aingle_memoize_init(&state.mem);
	state.links = NULL;

	/*
	 * Create the value implementations.
	 */

	aingle_resolved_writer_t  *result =
	    aingle_resolved_writer_new_memoized(&state, wschema, rschema);
	if (result == NULL) {
		aingle_memoize_done(&state.mem);
		return NULL;
	}

	/*
	 * Fix up any link schemas so that their value implementations
	 * point to their target schemas' implementations.
	 */

	aingle_resolved_writer_calculate_size(result);
	while (state.links != NULL) {
		aingle_resolved_link_writer_t  *liface = state.links;
		aingle_resolved_writer_calculate_size(liface->target_resolver);
		state.links = liface->next;
		liface->next = NULL;
	}

	/*
	 * And now we can return.
	 */

	aingle_memoize_done(&state.mem);
	return &result->parent;
}
