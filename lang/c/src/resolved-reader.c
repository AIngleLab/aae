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


typedef struct aingle_resolved_reader  aingle_resolved_reader_t;

struct aingle_resolved_reader {
	aingle_value_iface_t  parent;

	/** The reference count for this interface. */
	volatile int  refcount;

	/** The writer schema. */
	aingle_schema_t  wschema;

	/** The reader schema. */
	aingle_schema_t  rschema;

	/* The size of the value instances for this resolver. */
	size_t  instance_size;

	/* A function to calculate the instance size once the overall
	 * top-level resolver (and all of its children) have been
	 * constructed. */
	void
	(*calculate_size)(aingle_resolved_reader_t *iface);

	/* A free function for this resolver */
	void
	(*free_iface)(aingle_resolved_reader_t *iface, st_table *freeing);

	/* An initialization function for instances of this resolver. */
	int
	(*init)(const aingle_resolved_reader_t *iface, void *self);

	/* A finalization function for instances of this resolver. */
	void
	(*done)(const aingle_resolved_reader_t *iface, void *self);

	/* Clear out any existing wrappers, if any */
	int
	(*reset_wrappers)(const aingle_resolved_reader_t *iface, void *self);
};

#define aingle_resolved_reader_calculate_size(iface) \
	do { \
		if ((iface)->calculate_size != NULL) { \
			(iface)->calculate_size((iface)); \
		} \
	} while (0)
#define aingle_resolved_reader_init(iface, self) \
	((iface)->init == NULL? 0: (iface)->init((iface), (self)))
#define aingle_resolved_reader_done(iface, self) \
	((iface)->done == NULL? (void) 0: (iface)->done((iface), (self)))
#define aingle_resolved_reader_reset_wrappers(iface, self) \
	((iface)->reset_wrappers == NULL? 0: \
	 (iface)->reset_wrappers((iface), (self)))


/*
 * We assume that each instance type in this value contains an an
 * aingle_value_t as its first element, which is the current wrapped
 * value.
 */

void
aingle_resolved_reader_set_source(aingle_value_t *resolved,
				aingle_value_t *dest)
{
	aingle_value_t  *self = (aingle_value_t *) resolved->self;
	if (self->self != NULL) {
		aingle_value_decref(self);
	}
	aingle_value_copy_ref(self, dest);
}

void
aingle_resolved_reader_clear_source(aingle_value_t *resolved)
{
	aingle_value_t  *self = (aingle_value_t *) resolved->self;
	if (self->self != NULL) {
		aingle_value_decref(self);
	}
	self->iface = NULL;
	self->self = NULL;
}

int
aingle_resolved_reader_new_value(aingle_value_iface_t *viface,
			       aingle_value_t *value)
{
	int  rval;
	aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	void  *self = aingle_malloc(iface->instance_size + sizeof(volatile int));
	if (self == NULL) {
		value->iface = NULL;
		value->self = NULL;
		return ENOMEM;
	}

	memset(self, 0, iface->instance_size + sizeof(volatile int));
	volatile int  *refcount = (volatile int *) self;
	self = (char *) self + sizeof(volatile int);

	rval = aingle_resolved_reader_init(iface, self);
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
aingle_resolved_reader_free_value(const aingle_value_iface_t *viface, void *vself)
{
	aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;

	aingle_resolved_reader_done(iface, vself);
	if (self->self != NULL) {
		aingle_value_decref(self);
	}

	vself = (char *) vself - sizeof(volatile int);
	aingle_free(vself, iface->instance_size + sizeof(volatile int));
}

static void
aingle_resolved_reader_incref(aingle_value_t *value)
{
	/*
	 * This only works if you pass in the top-level value.
	 */

	volatile int  *refcount = (volatile int *) ((char *) value->self - sizeof(volatile int));
	aingle_refcount_inc(refcount);
}

static void
aingle_resolved_reader_decref(aingle_value_t *value)
{
	/*
	 * This only works if you pass in the top-level value.
	 */

	volatile int  *refcount = (volatile int *) ((char *) value->self - sizeof(volatile int));
	if (aingle_refcount_dec(refcount)) {
		aingle_resolved_reader_free_value(value->iface, value->self);
	}
}


static aingle_value_iface_t *
aingle_resolved_reader_incref_iface(aingle_value_iface_t *viface)
{
	aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	aingle_refcount_inc(&iface->refcount);
	return viface;
}

static void
free_resolver(aingle_resolved_reader_t *iface, st_table *freeing)
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
aingle_resolved_reader_calculate_size_(aingle_resolved_reader_t *iface)
{
	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_value_t);
}

static void
aingle_resolved_reader_free_iface(aingle_resolved_reader_t *iface, st_table *freeing)
{
	AINGLE_UNUSED(freeing);
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_reader_t, iface);
}

static void
aingle_resolved_reader_decref_iface(aingle_value_iface_t *viface)
{
	aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	DEBUG("Decref resolver %p (before=%d)", iface, iface->refcount);
	if (aingle_refcount_dec(&iface->refcount)) {
		st_table  *freeing = st_init_numtable();
		free_resolver(iface, freeing);
		st_free_table(freeing);
	}
}

static int
aingle_resolved_reader_reset(const aingle_value_iface_t *viface, void *vself)
{
	/*
	 * To reset a wrapped value, we first clear out any wrappers,
	 * and then have the wrapped value reset itself.
	 */

	int  rval;
	aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	aingle_value_t  *self = (aingle_value_t *) vself;
	check(rval, aingle_resolved_reader_reset_wrappers(iface, vself));
	return aingle_value_reset(self);
}

static aingle_type_t
aingle_resolved_reader_get_type(const aingle_value_iface_t *viface, const void *vself)
{
	AINGLE_UNUSED(vself);
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	return aingle_typeof(iface->rschema);
}

static aingle_schema_t
aingle_resolved_reader_get_schema(const aingle_value_iface_t *viface, const void *vself)
{
	AINGLE_UNUSED(vself);
	aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	return iface->rschema;
}


static aingle_resolved_reader_t *
aingle_resolved_reader_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_reader_t  *self = (aingle_resolved_reader_t  *) aingle_new(aingle_resolved_reader_t);
	memset(self, 0, sizeof(aingle_resolved_reader_t));

	self->parent.incref_iface = aingle_resolved_reader_incref_iface;
	self->parent.decref_iface = aingle_resolved_reader_decref_iface;
	self->parent.incref = aingle_resolved_reader_incref;
	self->parent.decref = aingle_resolved_reader_decref;
	self->parent.reset = aingle_resolved_reader_reset;
	self->parent.get_type = aingle_resolved_reader_get_type;
	self->parent.get_schema = aingle_resolved_reader_get_schema;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->calculate_size = aingle_resolved_reader_calculate_size_;
	self->free_iface = aingle_resolved_reader_free_iface;
	self->reset_wrappers = NULL;
	return self;
}


/*-----------------------------------------------------------------------
 * Memoized resolvers
 */

typedef struct aingle_resolved_link_reader  aingle_resolved_link_reader_t;

typedef struct memoize_state_t {
	aingle_memoize_t  mem;
	aingle_resolved_link_reader_t  *links;
} memoize_state_t;

static aingle_resolved_reader_t *
aingle_resolved_reader_new_memoized(memoize_state_t *state,
				  aingle_schema_t wschema, aingle_schema_t rschema);


/*-----------------------------------------------------------------------
 * Recursive schemas
 */

/*
 * Recursive schemas are handled specially; the value implementation for
 * an AINGLE_LINK schema is simply a wrapper around the value
 * implementation for the link's target schema.  The value methods all
 * delegate to the wrapped implementation.
 *
 * Complicating the links here is that we might be linking to the writer
 * schema or the reader schema.  This only matters for a couple of
 * methods, so instead of keeping a boolean flag in the value interface,
 * we just have separate method implementations that we slot in
 * appropriately.
 */

struct aingle_resolved_link_reader {
	aingle_resolved_reader_t  parent;

	/**
	 * A pointer to the “next” link resolver that we've had to
	 * create.  We use this as we're creating the overall top-level
	 * resolver to keep track of which ones we have to fix up
	 * afterwards.
	 */
	aingle_resolved_link_reader_t  *next;

	/** The target's implementation. */
	aingle_resolved_reader_t  *target_resolver;
};

typedef struct aingle_resolved_link_value {
	aingle_value_t  wrapped;
	aingle_value_t  target;
} aingle_resolved_link_value_t;

static void
aingle_resolved_wlink_reader_calculate_size(aingle_resolved_reader_t *iface)
{
	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for [%s]->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_resolved_link_value_t);
}

static void
aingle_resolved_rlink_reader_calculate_size(aingle_resolved_reader_t *iface)
{
	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->[%s]",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_resolved_link_value_t);
}

static void
aingle_resolved_link_reader_free_iface(aingle_resolved_reader_t *iface, st_table *freeing)
{
	aingle_resolved_link_reader_t  *liface =
	    container_of(iface, aingle_resolved_link_reader_t, parent);
	if (liface->target_resolver != NULL) {
		free_resolver(liface->target_resolver, freeing);
	}
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_link_reader_t, iface);
}

static int
aingle_resolved_link_reader_init(const aingle_resolved_reader_t *iface, void *vself)
{
	int  rval;
	const aingle_resolved_link_reader_t  *liface =
	    container_of(iface, aingle_resolved_link_reader_t, parent);
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

	rval = aingle_resolved_reader_init(liface->target_resolver, self->target.self);
	if (rval != 0) {
		aingle_free(self->target.self, target_instance_size);
	}
	return rval;
}

static void
aingle_resolved_link_reader_done(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_link_reader_t  *liface =
	    container_of(iface, aingle_resolved_link_reader_t, parent);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	size_t  target_instance_size = liface->target_resolver->instance_size;
	DEBUG("Freeing <%p:%" PRIsz "> for link", self->target.self, target_instance_size);
	aingle_resolved_reader_done(liface->target_resolver, self->target.self);
	aingle_free(self->target.self, target_instance_size);
	self->target.iface = NULL;
	self->target.self = NULL;
}

static int
aingle_resolved_link_reader_reset(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_link_reader_t  *liface =
	    container_of(iface, aingle_resolved_link_reader_t, parent);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	return aingle_resolved_reader_reset_wrappers
	    (liface->target_resolver, self->target.self);
}

static aingle_type_t
aingle_resolved_link_reader_get_type(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_type(&self->target);
}

static aingle_schema_t
aingle_resolved_link_reader_get_schema(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_schema(&self->target);
}

static int
aingle_resolved_link_reader_get_boolean(const aingle_value_iface_t *iface,
				      const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_boolean(&self->target, out);
}

static int
aingle_resolved_link_reader_get_bytes(const aingle_value_iface_t *iface,
				    const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_bytes(&self->target, buf, size);
}

static int
aingle_resolved_link_reader_grab_bytes(const aingle_value_iface_t *iface,
				     const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_grab_bytes(&self->target, dest);
}

static int
aingle_resolved_link_reader_get_double(const aingle_value_iface_t *iface,
				     const void *vself, double *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_double(&self->target, out);
}

static int
aingle_resolved_link_reader_get_float(const aingle_value_iface_t *iface,
				    const void *vself, float *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_float(&self->target, out);
}

static int
aingle_resolved_link_reader_get_int(const aingle_value_iface_t *iface,
				  const void *vself, int32_t *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_int(&self->target, out);
}

static int
aingle_resolved_link_reader_get_long(const aingle_value_iface_t *iface,
				   const void *vself, int64_t *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_long(&self->target, out);
}

static int
aingle_resolved_link_reader_get_null(const aingle_value_iface_t *iface, const void *vself)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_null(&self->target);
}

static int
aingle_resolved_link_reader_get_string(const aingle_value_iface_t *iface,
				     const void *vself, const char **str, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_string(&self->target, str, size);
}

static int
aingle_resolved_link_reader_grab_string(const aingle_value_iface_t *iface,
				      const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_grab_string(&self->target, dest);
}

static int
aingle_resolved_link_reader_get_enum(const aingle_value_iface_t *iface,
				   const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_enum(&self->target, out);
}

static int
aingle_resolved_link_reader_get_fixed(const aingle_value_iface_t *iface,
				    const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_fixed(&self->target, buf, size);
}

static int
aingle_resolved_link_reader_grab_fixed(const aingle_value_iface_t *iface,
				     const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_grab_fixed(&self->target, dest);
}

static int
aingle_resolved_link_reader_set_boolean(const aingle_value_iface_t *iface,
				      void *vself, int val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_boolean(&self->target, val);
}

static int
aingle_resolved_link_reader_set_bytes(const aingle_value_iface_t *iface,
				    void *vself, void *buf, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_bytes(&self->target, buf, size);
}

static int
aingle_resolved_link_reader_give_bytes(const aingle_value_iface_t *iface,
				     void *vself, aingle_wrapped_buffer_t *buf)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_give_bytes(&self->target, buf);
}

static int
aingle_resolved_link_reader_set_double(const aingle_value_iface_t *iface,
				     void *vself, double val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_double(&self->target, val);
}

static int
aingle_resolved_link_reader_set_float(const aingle_value_iface_t *iface,
				    void *vself, float val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_float(&self->target, val);
}

static int
aingle_resolved_link_reader_set_int(const aingle_value_iface_t *iface,
				  void *vself, int32_t val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_int(&self->target, val);
}

static int
aingle_resolved_link_reader_set_long(const aingle_value_iface_t *iface,
				   void *vself, int64_t val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_long(&self->target, val);
}

static int
aingle_resolved_link_reader_set_null(const aingle_value_iface_t *iface, void *vself)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_null(&self->target);
}

static int
aingle_resolved_link_reader_set_string(const aingle_value_iface_t *iface,
				     void *vself, const char *str)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_string(&self->target, str);
}

static int
aingle_resolved_link_reader_set_string_len(const aingle_value_iface_t *iface,
					 void *vself, const char *str, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_string_len(&self->target, str, size);
}

static int
aingle_resolved_link_reader_give_string_len(const aingle_value_iface_t *iface,
					  void *vself, aingle_wrapped_buffer_t *buf)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_give_string_len(&self->target, buf);
}

static int
aingle_resolved_link_reader_set_enum(const aingle_value_iface_t *iface,
				   void *vself, int val)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_enum(&self->target, val);
}

static int
aingle_resolved_link_reader_set_fixed(const aingle_value_iface_t *iface,
				    void *vself, void *buf, size_t size)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_fixed(&self->target, buf, size);
}

static int
aingle_resolved_link_reader_give_fixed(const aingle_value_iface_t *iface,
				     void *vself, aingle_wrapped_buffer_t *buf)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_give_fixed(&self->target, buf);
}

static int
aingle_resolved_link_reader_get_size(const aingle_value_iface_t *iface,
				   const void *vself, size_t *size)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_size(&self->target, size);
}

static int
aingle_resolved_link_reader_get_by_index(const aingle_value_iface_t *iface,
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
aingle_resolved_link_reader_get_by_name(const aingle_value_iface_t *iface,
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
aingle_resolved_link_reader_get_discriminant(const aingle_value_iface_t *iface,
					   const void *vself, int *out)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_discriminant(&self->target, out);
}

static int
aingle_resolved_link_reader_get_current_branch(const aingle_value_iface_t *iface,
					     const void *vself, aingle_value_t *branch)
{
	AINGLE_UNUSED(iface);
	const aingle_resolved_link_value_t  *self = (const aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_get_current_branch(&self->target, branch);
}

static int
aingle_resolved_link_reader_append(const aingle_value_iface_t *iface,
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
aingle_resolved_link_reader_add(const aingle_value_iface_t *iface,
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
aingle_resolved_link_reader_set_branch(const aingle_value_iface_t *iface,
				     void *vself, int discriminant,
				     aingle_value_t *branch)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_link_value_t  *self = (aingle_resolved_link_value_t *) vself;
	aingle_value_t  *target_vself = (aingle_value_t *) self->target.self;
	*target_vself = self->wrapped;
	return aingle_value_set_branch(&self->target, discriminant, branch);
}

static aingle_resolved_link_reader_t *
aingle_resolved_link_reader_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_reader_t  *self = (aingle_resolved_reader_t  *) aingle_new(aingle_resolved_link_reader_t);
	memset(self, 0, sizeof(aingle_resolved_link_reader_t));

	self->parent.incref_iface = aingle_resolved_reader_incref_iface;
	self->parent.decref_iface = aingle_resolved_reader_decref_iface;
	self->parent.incref = aingle_resolved_reader_incref;
	self->parent.decref = aingle_resolved_reader_decref;
	self->parent.reset = aingle_resolved_reader_reset;
	self->parent.get_type = aingle_resolved_link_reader_get_type;
	self->parent.get_schema = aingle_resolved_link_reader_get_schema;
	self->parent.get_size = aingle_resolved_link_reader_get_size;
	self->parent.get_by_index = aingle_resolved_link_reader_get_by_index;
	self->parent.get_by_name = aingle_resolved_link_reader_get_by_name;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->free_iface = aingle_resolved_link_reader_free_iface;
	self->init = aingle_resolved_link_reader_init;
	self->done = aingle_resolved_link_reader_done;
	self->reset_wrappers = aingle_resolved_link_reader_reset;

	self->parent.get_boolean = aingle_resolved_link_reader_get_boolean;
	self->parent.get_bytes = aingle_resolved_link_reader_get_bytes;
	self->parent.grab_bytes = aingle_resolved_link_reader_grab_bytes;
	self->parent.get_double = aingle_resolved_link_reader_get_double;
	self->parent.get_float = aingle_resolved_link_reader_get_float;
	self->parent.get_int = aingle_resolved_link_reader_get_int;
	self->parent.get_long = aingle_resolved_link_reader_get_long;
	self->parent.get_null = aingle_resolved_link_reader_get_null;
	self->parent.get_string = aingle_resolved_link_reader_get_string;
	self->parent.grab_string = aingle_resolved_link_reader_grab_string;
	self->parent.get_enum = aingle_resolved_link_reader_get_enum;
	self->parent.get_fixed = aingle_resolved_link_reader_get_fixed;
	self->parent.grab_fixed = aingle_resolved_link_reader_grab_fixed;

	self->parent.set_boolean = aingle_resolved_link_reader_set_boolean;
	self->parent.set_bytes = aingle_resolved_link_reader_set_bytes;
	self->parent.give_bytes = aingle_resolved_link_reader_give_bytes;
	self->parent.set_double = aingle_resolved_link_reader_set_double;
	self->parent.set_float = aingle_resolved_link_reader_set_float;
	self->parent.set_int = aingle_resolved_link_reader_set_int;
	self->parent.set_long = aingle_resolved_link_reader_set_long;
	self->parent.set_null = aingle_resolved_link_reader_set_null;
	self->parent.set_string = aingle_resolved_link_reader_set_string;
	self->parent.set_string_len = aingle_resolved_link_reader_set_string_len;
	self->parent.give_string_len = aingle_resolved_link_reader_give_string_len;
	self->parent.set_enum = aingle_resolved_link_reader_set_enum;
	self->parent.set_fixed = aingle_resolved_link_reader_set_fixed;
	self->parent.give_fixed = aingle_resolved_link_reader_give_fixed;

	self->parent.get_size = aingle_resolved_link_reader_get_size;
	self->parent.get_by_index = aingle_resolved_link_reader_get_by_index;
	self->parent.get_by_name = aingle_resolved_link_reader_get_by_name;
	self->parent.get_discriminant = aingle_resolved_link_reader_get_discriminant;
	self->parent.get_current_branch = aingle_resolved_link_reader_get_current_branch;

	self->parent.append = aingle_resolved_link_reader_append;
	self->parent.add = aingle_resolved_link_reader_add;
	self->parent.set_branch = aingle_resolved_link_reader_set_branch;

	return container_of(self, aingle_resolved_link_reader_t, parent);
}

static aingle_resolved_reader_t *
try_wlink(memoize_state_t *state,
	  aingle_schema_t wschema, aingle_schema_t rschema)
{
	AINGLE_UNUSED(rschema);

	/*
	 * For link schemas, we create a special value implementation
	 * that allocates space for its wrapped value at runtime.  This
	 * lets us handle recursive types without having to instantiate
	 * in infinite-size value.
	 */

	aingle_schema_t  wtarget = aingle_schema_link_target(wschema);
	aingle_resolved_link_reader_t  *lself =
	    aingle_resolved_link_reader_create(wtarget, rschema);
	aingle_memoize_set(&state->mem, wschema, rschema, lself);

	aingle_resolved_reader_t  *target_resolver =
	    aingle_resolved_reader_new_memoized(state, wtarget, rschema);
	if (target_resolver == NULL) {
		aingle_memoize_delete(&state->mem, wschema, rschema);
		aingle_value_iface_decref(&lself->parent.parent);
		aingle_prefix_error("Link target isn't compatible: ");
		DEBUG("%s", aingle_strerror());
		return NULL;
	}

	lself->parent.calculate_size = aingle_resolved_wlink_reader_calculate_size;
	lself->target_resolver = target_resolver;
	lself->next = state->links;
	state->links = lself;

	return &lself->parent;
}

static aingle_resolved_reader_t *
try_rlink(memoize_state_t *state,
	  aingle_schema_t wschema, aingle_schema_t rschema)
{
	AINGLE_UNUSED(rschema);

	/*
	 * For link schemas, we create a special value implementation
	 * that allocates space for its wrapped value at runtime.  This
	 * lets us handle recursive types without having to instantiate
	 * in infinite-size value.
	 */

	aingle_schema_t  rtarget = aingle_schema_link_target(rschema);
	aingle_resolved_link_reader_t  *lself =
	    aingle_resolved_link_reader_create(wschema, rtarget);
	aingle_memoize_set(&state->mem, wschema, rschema, lself);

	aingle_resolved_reader_t  *target_resolver =
	    aingle_resolved_reader_new_memoized(state, wschema, rtarget);
	if (target_resolver == NULL) {
		aingle_memoize_delete(&state->mem, wschema, rschema);
		aingle_value_iface_decref(&lself->parent.parent);
		aingle_prefix_error("Link target isn't compatible: ");
		DEBUG("%s", aingle_strerror());
		return NULL;
	}

	lself->parent.calculate_size = aingle_resolved_rlink_reader_calculate_size;
	lself->target_resolver = target_resolver;
	lself->next = state->links;
	state->links = lself;

	return &lself->parent;
}


/*-----------------------------------------------------------------------
 * boolean
 */

static int
aingle_resolved_reader_get_boolean(const aingle_value_iface_t *viface,
				 const void *vself, int *val)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting boolean from %p", src->self);
	return aingle_value_get_boolean(src, val);
}

static aingle_resolved_reader_t *
try_boolean(memoize_state_t *state,
	    aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_boolean(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_boolean = aingle_resolved_reader_get_boolean;
		return self;
	}
	aingle_set_error("Writer %s not compatible with reader boolean",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * bytes
 */

static int
aingle_resolved_reader_get_bytes(const aingle_value_iface_t *viface,
			       const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting bytes from %p", src->self);
	return aingle_value_get_bytes(src, buf, size);
}

static int
aingle_resolved_reader_grab_bytes(const aingle_value_iface_t *viface,
				const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Grabbing bytes from %p", src->self);
	return aingle_value_grab_bytes(src, dest);
}

static aingle_resolved_reader_t *
try_bytes(memoize_state_t *state,
	  aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_bytes(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_bytes = aingle_resolved_reader_get_bytes;
		self->parent.grab_bytes = aingle_resolved_reader_grab_bytes;
		return self;
	}
	aingle_set_error("Writer %s not compatible with reader bytes",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * double
 */

static int
aingle_resolved_reader_get_double(const aingle_value_iface_t *viface,
				const void *vself, double *val)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting double from %p", src->self);
	return aingle_value_get_double(src, val);
}

static int
aingle_resolved_reader_get_double_float(const aingle_value_iface_t *viface,
				      const void *vself, double *val)
{
	int  rval;
	float  real_val;
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Promoting double from float %p", src->self);
	check(rval, aingle_value_get_float(src, &real_val));
	*val = real_val;
	return 0;
}

static int
aingle_resolved_reader_get_double_int(const aingle_value_iface_t *viface,
				    const void *vself, double *val)
{
	int  rval;
	int32_t  real_val;
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Promoting double from int %p", src->self);
	check(rval, aingle_value_get_int(src, &real_val));
	*val = real_val;
	return 0;
}

static int
aingle_resolved_reader_get_double_long(const aingle_value_iface_t *viface,
				     const void *vself, double *val)
{
	int  rval;
	int64_t  real_val;
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Promoting double from long %p", src->self);
	check(rval, aingle_value_get_long(src, &real_val));
	*val = (double) real_val;
	return 0;
}

static aingle_resolved_reader_t *
try_double(memoize_state_t *state,
	   aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_double(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_double = aingle_resolved_reader_get_double;
		return self;
	}

	else if (is_aingle_float(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_double = aingle_resolved_reader_get_double_float;
		return self;
	}

	else if (is_aingle_int32(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_double = aingle_resolved_reader_get_double_int;
		return self;
	}

	else if (is_aingle_int64(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_double = aingle_resolved_reader_get_double_long;
		return self;
	}

	aingle_set_error("Writer %s not compatible with reader double",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * float
 */

static int
aingle_resolved_reader_get_float(const aingle_value_iface_t *viface,
			       const void *vself, float *val)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting float from %p", src->self);
	return aingle_value_get_float(src, val);
}

static int
aingle_resolved_reader_get_float_int(const aingle_value_iface_t *viface,
				   const void *vself, float *val)
{
	int  rval;
	int32_t  real_val;
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Promoting float from int %p", src->self);
	check(rval, aingle_value_get_int(src, &real_val));
	*val = (float) real_val;
	return 0;
}

static int
aingle_resolved_reader_get_float_long(const aingle_value_iface_t *viface,
				    const void *vself, float *val)
{
	int  rval;
	int64_t  real_val;
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Promoting float from long %p", src->self);
	check(rval, aingle_value_get_long(src, &real_val));
	*val = (float) real_val;
	return 0;
}

static aingle_resolved_reader_t *
try_float(memoize_state_t *state,
	  aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_float(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_float = aingle_resolved_reader_get_float;
		return self;
	}

	else if (is_aingle_int32(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_float = aingle_resolved_reader_get_float_int;
		return self;
	}

	else if (is_aingle_int64(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_float = aingle_resolved_reader_get_float_long;
		return self;
	}

	aingle_set_error("Writer %s not compatible with reader float",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * int
 */

static int
aingle_resolved_reader_get_int(const aingle_value_iface_t *viface,
			     const void *vself, int32_t *val)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting int from %p", src->self);
	return aingle_value_get_int(src, val);
}

static aingle_resolved_reader_t *
try_int(memoize_state_t *state,
	aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_int32(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_int = aingle_resolved_reader_get_int;
		return self;
	}
	aingle_set_error("Writer %s not compatible with reader int",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * long
 */

static int
aingle_resolved_reader_get_long(const aingle_value_iface_t *viface,
			      const void *vself, int64_t *val)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting long from %p", src->self);
	return aingle_value_get_long(src, val);
}

static int
aingle_resolved_reader_get_long_int(const aingle_value_iface_t *viface,
				  const void *vself, int64_t *val)
{
	int  rval;
	int32_t  real_val;
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Promoting long from int %p", src->self);
	check(rval, aingle_value_get_int(src, &real_val));
	*val = real_val;
	return 0;
}

static aingle_resolved_reader_t *
try_long(memoize_state_t *state,
	 aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_int64(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_long = aingle_resolved_reader_get_long;
		return self;
	}

	else if (is_aingle_int32(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_long = aingle_resolved_reader_get_long_int;
		return self;
	}

	aingle_set_error("Writer %s not compatible with reader long",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * null
 */

static int
aingle_resolved_reader_get_null(const aingle_value_iface_t *viface,
			      const void *vself)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting null from %p", src->self);
	return aingle_value_get_null(src);
}

static aingle_resolved_reader_t *
try_null(memoize_state_t *state,
	 aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_null(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_null = aingle_resolved_reader_get_null;
		return self;
	}
	aingle_set_error("Writer %s not compatible with reader null",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * string
 */

static int
aingle_resolved_reader_get_string(const aingle_value_iface_t *viface,
				const void *vself, const char **str, size_t *size)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting string from %p", src->self);
	return aingle_value_get_string(src, str, size);
}

static int
aingle_resolved_reader_grab_string(const aingle_value_iface_t *viface,
				 const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Grabbing string from %p", src->self);
	return aingle_value_grab_string(src, dest);
}

static aingle_resolved_reader_t *
try_string(memoize_state_t *state,
	   aingle_schema_t wschema, aingle_schema_t rschema)
{
	if (is_aingle_string(wschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_string = aingle_resolved_reader_get_string;
		self->parent.grab_string = aingle_resolved_reader_grab_string;
		return self;
	}
	aingle_set_error("Writer %s not compatible with reader string",
		       aingle_schema_type_name(wschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * array
 */

typedef struct aingle_resolved_array_reader {
	aingle_resolved_reader_t  parent;
	aingle_resolved_reader_t  *child_resolver;
} aingle_resolved_array_reader_t;

typedef struct aingle_resolved_array_value {
	aingle_value_t  wrapped;
	aingle_raw_array_t  children;
} aingle_resolved_array_value_t;

static void
aingle_resolved_array_reader_calculate_size(aingle_resolved_reader_t *iface)
{
	aingle_resolved_array_reader_t  *aiface =
	    container_of(iface, aingle_resolved_array_reader_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_resolved_array_value_t);

	aingle_resolved_reader_calculate_size(aiface->child_resolver);
}

static void
aingle_resolved_array_reader_free_iface(aingle_resolved_reader_t *iface, st_table *freeing)
{
	aingle_resolved_array_reader_t  *aiface =
	    container_of(iface, aingle_resolved_array_reader_t, parent);
	free_resolver(aiface->child_resolver, freeing);
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_array_reader_t, iface);
}

static int
aingle_resolved_array_reader_init(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_array_reader_t  *aiface =
	    container_of(iface, aingle_resolved_array_reader_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;
	size_t  child_instance_size = aiface->child_resolver->instance_size;
	DEBUG("Initializing child array (child_size=%" PRIsz ")", child_instance_size);
	aingle_raw_array_init(&self->children, child_instance_size);
	return 0;
}

static void
aingle_resolved_array_reader_free_elements(const aingle_resolved_reader_t *child_iface,
					 aingle_resolved_array_value_t *self)
{
	size_t  i;
	for (i = 0; i < aingle_raw_array_size(&self->children); i++) {
		void  *child_self = aingle_raw_array_get_raw(&self->children, i);
		aingle_resolved_reader_done(child_iface, child_self);
	}
}

static void
aingle_resolved_array_reader_done(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_array_reader_t  *aiface =
	    container_of(iface, aingle_resolved_array_reader_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;
	aingle_resolved_array_reader_free_elements(aiface->child_resolver, self);
	aingle_raw_array_done(&self->children);
}

static int
aingle_resolved_array_reader_reset(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_array_reader_t  *aiface =
	    container_of(iface, aingle_resolved_array_reader_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;

	/* Clear out our cache of wrapped children */
	aingle_resolved_array_reader_free_elements(aiface->child_resolver, self);
	aingle_raw_array_clear(&self->children);
	return 0;
}

static int
aingle_resolved_array_reader_get_size(const aingle_value_iface_t *viface,
				    const void *vself, size_t *size)
{
	AINGLE_UNUSED(viface);
	const aingle_resolved_array_value_t  *self = (const aingle_resolved_array_value_t *) vself;
	return aingle_value_get_size(&self->wrapped, size);
}

static int
aingle_resolved_array_reader_get_by_index(const aingle_value_iface_t *viface,
					const void *vself, size_t index,
					aingle_value_t *child, const char **name)
{
	int  rval;
	size_t  old_size;
	size_t  new_size;
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_array_reader_t  *aiface =
	    container_of(iface, aingle_resolved_array_reader_t, parent);
	aingle_resolved_array_value_t  *self = (aingle_resolved_array_value_t *) vself;

	/*
	 * Ensure that our child wrapper array is big enough to hold
	 * this many elements.
	 */
	new_size = index + 1;
	check(rval, aingle_raw_array_ensure_size0(&self->children, new_size));
	old_size = aingle_raw_array_size(&self->children);
	if (old_size <= index) {
		size_t  i;
		for (i = old_size; i < new_size; i++) {
			check(rval, aingle_resolved_reader_init
			      (aiface->child_resolver,
			       aingle_raw_array_get_raw(&self->children, i)));
		}
		aingle_raw_array_size(&self->children) = index+1;
	}

	child->iface = &aiface->child_resolver->parent;
	child->self = aingle_raw_array_get_raw(&self->children, index);

	DEBUG("Getting element %" PRIsz " from array %p", index, self->wrapped.self);
	return aingle_value_get_by_index(&self->wrapped, index, (aingle_value_t *) child->self, name);
}

static aingle_resolved_array_reader_t *
aingle_resolved_array_reader_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_reader_t  *self = (aingle_resolved_reader_t  *) aingle_new(aingle_resolved_array_reader_t);
	memset(self, 0, sizeof(aingle_resolved_array_reader_t));

	self->parent.incref_iface = aingle_resolved_reader_incref_iface;
	self->parent.decref_iface = aingle_resolved_reader_decref_iface;
	self->parent.incref = aingle_resolved_reader_incref;
	self->parent.decref = aingle_resolved_reader_decref;
	self->parent.reset = aingle_resolved_reader_reset;
	self->parent.get_type = aingle_resolved_reader_get_type;
	self->parent.get_schema = aingle_resolved_reader_get_schema;
	self->parent.get_size = aingle_resolved_array_reader_get_size;
	self->parent.get_by_index = aingle_resolved_array_reader_get_by_index;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->calculate_size = aingle_resolved_array_reader_calculate_size;
	self->free_iface = aingle_resolved_array_reader_free_iface;
	self->init = aingle_resolved_array_reader_init;
	self->done = aingle_resolved_array_reader_done;
	self->reset_wrappers = aingle_resolved_array_reader_reset;
	return container_of(self, aingle_resolved_array_reader_t, parent);
}

static aingle_resolved_reader_t *
try_array(memoize_state_t *state,
	  aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * First verify that the writer is an array.
	 */

	if (!is_aingle_array(wschema)) {
		return 0;
	}

	/*
	 * Array schemas have to have compatible element schemas to be
	 * compatible themselves.  Try to create an resolver to check
	 * the compatibility.
	 */

	aingle_resolved_array_reader_t  *aself =
	    aingle_resolved_array_reader_create(wschema, rschema);
	aingle_memoize_set(&state->mem, wschema, rschema, aself);

	aingle_schema_t  witems = aingle_schema_array_items(wschema);
	aingle_schema_t  ritems = aingle_schema_array_items(rschema);

	aingle_resolved_reader_t  *item_resolver =
	    aingle_resolved_reader_new_memoized(state, witems, ritems);
	if (item_resolver == NULL) {
		aingle_memoize_delete(&state->mem, wschema, rschema);
		aingle_value_iface_decref(&aself->parent.parent);
		aingle_prefix_error("Array values aren't compatible: ");
		return NULL;
	}

	/*
	 * The two schemas are compatible.  Store the item schema's
	 * resolver into the child_resolver field.
	 */

	aself->child_resolver = item_resolver;
	return &aself->parent;
}


/*-----------------------------------------------------------------------
 * enum
 */

static int
aingle_resolved_reader_get_enum(const aingle_value_iface_t *viface,
			      const void *vself, int *val)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting enum from %p", src->self);
	return aingle_value_get_enum(src, val);
}

static aingle_resolved_reader_t *
try_enum(memoize_state_t *state,
	 aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * Enum schemas have to have the same name — but not the same
	 * list of symbols — to be compatible.
	 */

	if (is_aingle_enum(wschema)) {
		const char  *wname = aingle_schema_name(wschema);
		const char  *rname = aingle_schema_name(rschema);

		if (strcmp(wname, rname) == 0) {
			aingle_resolved_reader_t  *self =
			    aingle_resolved_reader_create(wschema, rschema);
			aingle_memoize_set(&state->mem, wschema, rschema, self);
			self->parent.get_enum = aingle_resolved_reader_get_enum;
			return self;
		}
	}
	aingle_set_error("Writer %s not compatible with reader %s",
		       aingle_schema_type_name(wschema),
		       aingle_schema_type_name(rschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * fixed
 */

static int
aingle_resolved_reader_get_fixed(const aingle_value_iface_t *viface,
			       const void *vself, const void **buf, size_t *size)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Getting fixed from %p", vself);
	return aingle_value_get_fixed(src, buf, size);
}

static int
aingle_resolved_reader_grab_fixed(const aingle_value_iface_t *viface,
				const void *vself, aingle_wrapped_buffer_t *dest)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	DEBUG("Grabbing fixed from %p", vself);
	return aingle_value_grab_fixed(src, dest);
}

static aingle_resolved_reader_t *
try_fixed(memoize_state_t *state,
	  aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * Fixed schemas need the same name and size to be compatible.
	 */

	if (aingle_schema_equal(wschema, rschema)) {
		aingle_resolved_reader_t  *self =
		    aingle_resolved_reader_create(wschema, rschema);
		aingle_memoize_set(&state->mem, wschema, rschema, self);
		self->parent.get_fixed = aingle_resolved_reader_get_fixed;
		self->parent.grab_fixed = aingle_resolved_reader_grab_fixed;
		return self;
	}
	aingle_set_error("Writer %s not compatible with reader %s",
		       aingle_schema_type_name(wschema),
		       aingle_schema_type_name(rschema));
	return NULL;
}


/*-----------------------------------------------------------------------
 * map
 */

typedef struct aingle_resolved_map_reader {
	aingle_resolved_reader_t  parent;
	aingle_resolved_reader_t  *child_resolver;
} aingle_resolved_map_reader_t;

typedef struct aingle_resolved_map_value {
	aingle_value_t  wrapped;
	aingle_raw_array_t  children;
} aingle_resolved_map_value_t;

static void
aingle_resolved_map_reader_calculate_size(aingle_resolved_reader_t *iface)
{
	aingle_resolved_map_reader_t  *miface =
	    container_of(iface, aingle_resolved_map_reader_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));
	iface->instance_size = sizeof(aingle_resolved_map_value_t);

	aingle_resolved_reader_calculate_size(miface->child_resolver);
}

static void
aingle_resolved_map_reader_free_iface(aingle_resolved_reader_t *iface, st_table *freeing)
{
	aingle_resolved_map_reader_t  *miface =
	    container_of(iface, aingle_resolved_map_reader_t, parent);
	free_resolver(miface->child_resolver, freeing);
	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_map_reader_t, iface);
}

static int
aingle_resolved_map_reader_init(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_map_reader_t  *miface =
	    container_of(iface, aingle_resolved_map_reader_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;
	size_t  child_instance_size = miface->child_resolver->instance_size;
	DEBUG("Initializing child array for map (child_size=%" PRIsz ")", child_instance_size);
	aingle_raw_array_init(&self->children, child_instance_size);
	return 0;
}

static void
aingle_resolved_map_reader_free_elements(const aingle_resolved_reader_t *child_iface,
				       aingle_resolved_map_value_t *self)
{
	size_t  i;
	for (i = 0; i < aingle_raw_array_size(&self->children); i++) {
		void  *child_self = aingle_raw_array_get_raw(&self->children, i);
		aingle_resolved_reader_done(child_iface, child_self);
	}
}

static void
aingle_resolved_map_reader_done(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_map_reader_t  *miface =
	    container_of(iface, aingle_resolved_map_reader_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;
	aingle_resolved_map_reader_free_elements(miface->child_resolver, self);
	aingle_raw_array_done(&self->children);
}

static int
aingle_resolved_map_reader_reset(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_map_reader_t  *miface =
	    container_of(iface, aingle_resolved_map_reader_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;

	/* Clear out our cache of wrapped children */
	aingle_resolved_map_reader_free_elements(miface->child_resolver, self);
	return 0;
}

static int
aingle_resolved_map_reader_get_size(const aingle_value_iface_t *viface,
				  const void *vself, size_t *size)
{
	AINGLE_UNUSED(viface);
	const aingle_value_t  *src = (const aingle_value_t *) vself;
	return aingle_value_get_size(src, size);
}

static int
aingle_resolved_map_reader_get_by_index(const aingle_value_iface_t *viface,
				      const void *vself, size_t index,
				      aingle_value_t *child, const char **name)
{
	int  rval;
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_map_reader_t  *miface =
	    container_of(iface, aingle_resolved_map_reader_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;

	/*
	 * Ensure that our child wrapper array is big enough to hold
	 * this many elements.
	 */
	check(rval, aingle_raw_array_ensure_size0(&self->children, index+1));
	if (aingle_raw_array_size(&self->children) <= index) {
		aingle_raw_array_size(&self->children) = index+1;
	}

	child->iface = &miface->child_resolver->parent;
	child->self = aingle_raw_array_get_raw(&self->children, index);

	DEBUG("Getting element %" PRIsz " from map %p", index, self->wrapped.self);
	return aingle_value_get_by_index(&self->wrapped, index, (aingle_value_t *) child->self, name);
}

static int
aingle_resolved_map_reader_get_by_name(const aingle_value_iface_t *viface,
				     const void *vself, const char *name,
				     aingle_value_t *child, size_t *index)
{
	int  rval;
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_map_reader_t  *miface =
	    container_of(iface, aingle_resolved_map_reader_t, parent);
	aingle_resolved_map_value_t  *self = (aingle_resolved_map_value_t *) vself;

	/*
	 * This is a bit convoluted.  We need to stash the wrapped child
	 * value somewhere in our children array.  But we don't know
	 * where to put it until the wrapped map tells us what its index
	 * is.
	 */

	aingle_value_t  real_child;
	size_t  real_index;

	DEBUG("Getting element %s from map %p", name, self->wrapped.self);
	check(rval, aingle_value_get_by_name
	      (&self->wrapped, name, &real_child, &real_index));

	/*
	 * Ensure that our child wrapper array is big enough to hold
	 * this many elements.
	 */
	check(rval, aingle_raw_array_ensure_size0(&self->children, real_index+1));
	if (aingle_raw_array_size(&self->children) <= real_index) {
		aingle_raw_array_size(&self->children) = real_index+1;
	}

	child->iface = &miface->child_resolver->parent;
	child->self = aingle_raw_array_get_raw(&self->children, real_index);
	aingle_value_t  *child_vself = (aingle_value_t *) child->self;
	*child_vself = real_child;

	if (index != NULL) {
		*index = real_index;
	}
	return 0;
}

static aingle_resolved_map_reader_t *
aingle_resolved_map_reader_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_reader_t  *self = (aingle_resolved_reader_t *) aingle_new(aingle_resolved_map_reader_t);
	memset(self, 0, sizeof(aingle_resolved_map_reader_t));

	self->parent.incref_iface = aingle_resolved_reader_incref_iface;
	self->parent.decref_iface = aingle_resolved_reader_decref_iface;
	self->parent.incref = aingle_resolved_reader_incref;
	self->parent.decref = aingle_resolved_reader_decref;
	self->parent.reset = aingle_resolved_reader_reset;
	self->parent.get_type = aingle_resolved_reader_get_type;
	self->parent.get_schema = aingle_resolved_reader_get_schema;
	self->parent.get_size = aingle_resolved_map_reader_get_size;
	self->parent.get_by_index = aingle_resolved_map_reader_get_by_index;
	self->parent.get_by_name = aingle_resolved_map_reader_get_by_name;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->calculate_size = aingle_resolved_map_reader_calculate_size;
	self->free_iface = aingle_resolved_map_reader_free_iface;
	self->init = aingle_resolved_map_reader_init;
	self->done = aingle_resolved_map_reader_done;
	self->reset_wrappers = aingle_resolved_map_reader_reset;
	return container_of(self, aingle_resolved_map_reader_t, parent);
}

static aingle_resolved_reader_t *
try_map(memoize_state_t *state,
	aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * First verify that the reader is an map.
	 */

	if (!is_aingle_map(wschema)) {
		return 0;
	}

	/*
	 * Map schemas have to have compatible element schemas to be
	 * compatible themselves.  Try to create an resolver to check
	 * the compatibility.
	 */

	aingle_resolved_map_reader_t  *mself =
	    aingle_resolved_map_reader_create(wschema, rschema);
	aingle_memoize_set(&state->mem, wschema, rschema, mself);

	aingle_schema_t  witems = aingle_schema_map_values(wschema);
	aingle_schema_t  ritems = aingle_schema_map_values(rschema);

	aingle_resolved_reader_t  *item_resolver =
	    aingle_resolved_reader_new_memoized(state, witems, ritems);
	if (item_resolver == NULL) {
		aingle_memoize_delete(&state->mem, wschema, rschema);
		aingle_value_iface_decref(&mself->parent.parent);
		aingle_prefix_error("Map values aren't compatible: ");
		return NULL;
	}

	/*
	 * The two schemas are compatible.  Store the item schema's
	 * resolver into the child_resolver field.
	 */

	mself->child_resolver = item_resolver;
	return &mself->parent;
}


/*-----------------------------------------------------------------------
 * record
 */

typedef struct aingle_resolved_record_reader {
	aingle_resolved_reader_t  parent;
	size_t  field_count;
	size_t  *field_offsets;
	aingle_resolved_reader_t  **field_resolvers;
	size_t  *index_mapping;
} aingle_resolved_record_reader_t;

typedef struct aingle_resolved_record_value {
	aingle_value_t  wrapped;
	/* The rest of the struct is taken up by the inline storage
	 * needed for each field. */
} aingle_resolved_record_value_t;

/** Return a pointer to the given field within a record struct. */
#define aingle_resolved_record_field(iface, rec, index) \
	(((char *) (rec)) + (iface)->field_offsets[(index)])


static void
aingle_resolved_record_reader_calculate_size(aingle_resolved_reader_t *iface)
{
	aingle_resolved_record_reader_t  *riface =
	    container_of(iface, aingle_resolved_record_reader_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));

	/*
	 * Once we've figured out which reader fields we actually need,
	 * calculate an offset for each one.
	 */

	size_t  ri;
	size_t  next_offset = sizeof(aingle_resolved_record_value_t);
	for (ri = 0; ri < riface->field_count; ri++) {
		riface->field_offsets[ri] = next_offset;
		if (riface->field_resolvers[ri] != NULL) {
			aingle_resolved_reader_calculate_size
			    (riface->field_resolvers[ri]);
			size_t  field_size =
			    riface->field_resolvers[ri]->instance_size;
			DEBUG("Field %" PRIsz " has size %" PRIsz, ri, field_size);
			next_offset += field_size;
		} else {
			DEBUG("Field %" PRIsz " is being skipped", ri);
		}
	}

	DEBUG("Record has size %" PRIsz, next_offset);
	iface->instance_size = next_offset;
}


static void
aingle_resolved_record_reader_free_iface(aingle_resolved_reader_t *iface, st_table *freeing)
{
	aingle_resolved_record_reader_t  *riface =
	    container_of(iface, aingle_resolved_record_reader_t, parent);

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
			  riface->field_count * sizeof(aingle_resolved_reader_t *));
	}

	if (riface->index_mapping != NULL) {
		aingle_free(riface->index_mapping,
			  riface->field_count * sizeof(size_t));
	}

	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_record_reader_t, iface);
}

static int
aingle_resolved_record_reader_init(const aingle_resolved_reader_t *iface, void *vself)
{
	int  rval;
	const aingle_resolved_record_reader_t  *riface =
	    container_of(iface, aingle_resolved_record_reader_t, parent);
	aingle_resolved_record_value_t  *self = (aingle_resolved_record_value_t *) vself;

	/* Initialize each field */
	size_t  i;
	for (i = 0; i < riface->field_count; i++) {
		if (riface->field_resolvers[i] != NULL) {
			check(rval, aingle_resolved_reader_init
			      (riface->field_resolvers[i],
			       aingle_resolved_record_field(riface, self, i)));
		}
	}

	return 0;
}

static void
aingle_resolved_record_reader_done(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_record_reader_t  *riface =
	    container_of(iface, aingle_resolved_record_reader_t, parent);
	aingle_resolved_record_value_t  *self = (aingle_resolved_record_value_t  *) vself;

	/* Finalize each field */
	size_t  i;
	for (i = 0; i < riface->field_count; i++) {
		if (riface->field_resolvers[i] != NULL) {
			aingle_resolved_reader_done
			    (riface->field_resolvers[i],
			     aingle_resolved_record_field(riface, self, i));
		}
	}
}

static int
aingle_resolved_record_reader_reset(const aingle_resolved_reader_t *iface, void *vself)
{
	int  rval;
	const aingle_resolved_record_reader_t  *riface =
	    container_of(iface, aingle_resolved_record_reader_t, parent);
	aingle_resolved_record_value_t  *self = (aingle_resolved_record_value_t *) vself;

	/* Reset each field */
	size_t  i;
	for (i = 0; i < riface->field_count; i++) {
		if (riface->field_resolvers[i] != NULL) {
			check(rval, aingle_resolved_reader_reset_wrappers
			      (riface->field_resolvers[i],
			       aingle_resolved_record_field(riface, self, i)));
		}
	}

	return 0;
}

static int
aingle_resolved_record_reader_get_size(const aingle_value_iface_t *viface,
				     const void *vself, size_t *size)
{
	AINGLE_UNUSED(vself);
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_record_reader_t  *riface =
	    container_of(iface, aingle_resolved_record_reader_t, parent);
	*size = riface->field_count;
	return 0;
}

static int
aingle_resolved_record_reader_get_by_index(const aingle_value_iface_t *viface,
					 const void *vself, size_t index,
					 aingle_value_t *child, const char **name)
{
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_record_reader_t  *riface =
	    container_of(iface, aingle_resolved_record_reader_t, parent);
	const aingle_resolved_record_value_t  *self = (aingle_resolved_record_value_t *) vself;

	DEBUG("Getting reader field %" PRIsz " from record %p", index, self->wrapped.self);
	if (riface->field_resolvers[index] == NULL) {
		/*
		 * TODO: Return the default value if the writer record
		 * doesn't contain this field.
		 */
		DEBUG("Writer doesn't have field");
		aingle_set_error("NIY: Default values");
		return EINVAL;
	}

	size_t  writer_index = riface->index_mapping[index];
	DEBUG("  Writer field is %" PRIsz, writer_index);
	child->iface = &riface->field_resolvers[index]->parent;
	child->self = aingle_resolved_record_field(riface, self, index);
	return aingle_value_get_by_index(&self->wrapped, writer_index, (aingle_value_t *) child->self, name);
}

static int
aingle_resolved_record_reader_get_by_name(const aingle_value_iface_t *viface,
					const void *vself, const char *name,
					aingle_value_t *child, size_t *index)
{
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);

	int  ri = aingle_schema_record_field_get_index(iface->rschema, name);
	if (ri == -1) {
		aingle_set_error("Record doesn't have field named %s", name);
		return EINVAL;
	}

	DEBUG("Reader field %s is at index %d", name, ri);
	if (index != NULL) {
		*index = ri;
	}
	return aingle_resolved_record_reader_get_by_index(viface, vself, ri, child, NULL);
}

static aingle_resolved_record_reader_t *
aingle_resolved_record_reader_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_reader_t  *self = (aingle_resolved_reader_t *) aingle_new(aingle_resolved_record_reader_t);
	memset(self, 0, sizeof(aingle_resolved_record_reader_t));

	self->parent.incref_iface = aingle_resolved_reader_incref_iface;
	self->parent.decref_iface = aingle_resolved_reader_decref_iface;
	self->parent.incref = aingle_resolved_reader_incref;
	self->parent.decref = aingle_resolved_reader_decref;
	self->parent.reset = aingle_resolved_reader_reset;
	self->parent.get_type = aingle_resolved_reader_get_type;
	self->parent.get_schema = aingle_resolved_reader_get_schema;
	self->parent.get_size = aingle_resolved_record_reader_get_size;
	self->parent.get_by_index = aingle_resolved_record_reader_get_by_index;
	self->parent.get_by_name = aingle_resolved_record_reader_get_by_name;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->calculate_size = aingle_resolved_record_reader_calculate_size;
	self->free_iface = aingle_resolved_record_reader_free_iface;
	self->init = aingle_resolved_record_reader_init;
	self->done = aingle_resolved_record_reader_done;
	self->reset_wrappers = aingle_resolved_record_reader_reset;
	return container_of(self, aingle_resolved_record_reader_t, parent);
}

static aingle_resolved_reader_t *
try_record(memoize_state_t *state,
	   aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * First verify that the writer is also a record, and has the
	 * same name as the reader.
	 */

	if (!is_aingle_record(wschema)) {
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
	 * for each field in the reader schema.  To build this array, we
	 * loop through the fields of the reader schema.  If that field
	 * is also in the writer schema, we resolve them recursively,
	 * and store the resolver into the array.  If the field isn't in
	 * the writer schema, we raise an error.  (TODO: Eventually,
	 * we'll handle default values here.)  After this loop finishes,
	 * any NULLs in the field_resolvers array will represent fields
	 * in the writer but not the reader; these fields should be
	 * skipped, and won't be accessible in the resolved reader.
	 */

	aingle_resolved_record_reader_t  *rself =
	    aingle_resolved_record_reader_create(wschema, rschema);
	aingle_memoize_set(&state->mem, wschema, rschema, rself);

	size_t  rfields = aingle_schema_record_size(rschema);

	DEBUG("Checking reader record schema %s", wname);

	aingle_resolved_reader_t  **field_resolvers =
	    (aingle_resolved_reader_t **) aingle_calloc(rfields, sizeof(aingle_resolved_reader_t *));
	size_t  *field_offsets = (size_t *) aingle_calloc(rfields, sizeof(size_t));
	size_t  *index_mapping = (size_t *) aingle_calloc(rfields, sizeof(size_t));

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
		aingle_resolved_reader_t  *field_resolver =
		    aingle_resolved_reader_new_memoized(state, wfield, rfield);

		if (field_resolver == NULL) {
			aingle_prefix_error("Field %s isn't compatible: ", field_name);
			goto error;
		}

		/*
		 * Save the details for this field.
		 */

		DEBUG("Found match for field %s (%" PRIsz " in reader, %d in writer)",
		      field_name, ri, wi);
		field_resolvers[ri] = field_resolver;
		index_mapping[ri] = wi;
	}

	/*
	 * We might not have found matches for all of the writer fields,
	 * but that's okay — any extras will be ignored.
	 */

	rself->field_count = rfields;
	rself->field_offsets = field_offsets;
	rself->field_resolvers = field_resolvers;
	rself->index_mapping = index_mapping;
	return &rself->parent;

error:
	/*
	 * Clean up any resolver we might have already created.
	 */

	aingle_memoize_delete(&state->mem, wschema, rschema);
	aingle_value_iface_decref(&rself->parent.parent);

	{
		unsigned int  i;
		for (i = 0; i < rfields; i++) {
			if (field_resolvers[i]) {
				aingle_value_iface_decref(&field_resolvers[i]->parent);
			}
		}
	}

	aingle_free(field_resolvers, rfields * sizeof(aingle_resolved_reader_t *));
	aingle_free(field_offsets, rfields * sizeof(size_t));
	aingle_free(index_mapping, rfields * sizeof(size_t));
	return NULL;
}


/*-----------------------------------------------------------------------
 * writer union
 */

/*
 * For writer unions, we maintain a list of resolvers for each branch of
 * the union.  When we encounter a writer value, we see which branch it
 * is, and choose a reader resolver based on that.
 */

typedef struct aingle_resolved_wunion_reader {
	aingle_resolved_reader_t  parent;

	/* The number of branches in the writer union */
	size_t  branch_count;

	/* A child resolver for each branch of the writer union.  If any
	 * of these are NULL, then we don't have anything on the reader
	 * side that's compatible with that writer branch. */
	aingle_resolved_reader_t  **branch_resolvers;

} aingle_resolved_wunion_reader_t;

typedef struct aingle_resolved_wunion_value {
	aingle_value_t  wrapped;

	/** The currently active branch of the union.  -1 if no branch
	 * is selected. */
	int  discriminant;

	/* The rest of the struct is taken up by the inline storage
	 * needed for the active branch. */
} aingle_resolved_wunion_value_t;

/** Return a pointer to the active branch within a union struct. */
#define aingle_resolved_wunion_branch(_wunion) \
	(((char *) (_wunion)) + sizeof(aingle_resolved_wunion_value_t))


static void
aingle_resolved_wunion_reader_calculate_size(aingle_resolved_reader_t *iface)
{
	aingle_resolved_wunion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_wunion_reader_t, parent);

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
			aingle_resolved_reader_calculate_size
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
	    sizeof(aingle_resolved_wunion_value_t) + max_branch_size;
	DEBUG("Total union size is %" PRIsz, iface->instance_size);
}


static void
aingle_resolved_wunion_reader_free_iface(aingle_resolved_reader_t *iface, st_table *freeing)
{
	aingle_resolved_wunion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_wunion_reader_t, parent);

	if (uiface->branch_resolvers != NULL) {
		size_t  i;
		for (i = 0; i < uiface->branch_count; i++) {
			if (uiface->branch_resolvers[i] != NULL) {
				free_resolver(uiface->branch_resolvers[i], freeing);
			}
		}
		aingle_free(uiface->branch_resolvers,
			  uiface->branch_count * sizeof(aingle_resolved_reader_t *));
	}

	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_wunion_reader_t, iface);
}

static int
aingle_resolved_wunion_reader_init(const aingle_resolved_reader_t *iface, void *vself)
{
	AINGLE_UNUSED(iface);
	aingle_resolved_wunion_value_t  *self = (aingle_resolved_wunion_value_t *) vself;
	self->discriminant = -1;
	return 0;
}

static void
aingle_resolved_wunion_reader_done(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_wunion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_wunion_reader_t, parent);
	aingle_resolved_wunion_value_t  *self = (aingle_resolved_wunion_value_t *) vself;
	if (self->discriminant >= 0) {
		aingle_resolved_reader_done
		    (uiface->branch_resolvers[self->discriminant],
		     aingle_resolved_wunion_branch(self));
		self->discriminant = -1;
	}
}

static int
aingle_resolved_wunion_reader_reset(const aingle_resolved_reader_t *iface, void *vself)
{
	const aingle_resolved_wunion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_wunion_reader_t, parent);
	aingle_resolved_wunion_value_t  *self = (aingle_resolved_wunion_value_t *) vself;

	/* Keep the same branch selected, for the common case that we're
	 * about to reuse it. */
	if (self->discriminant >= 0) {
		return aingle_resolved_reader_reset_wrappers
		    (uiface->branch_resolvers[self->discriminant],
		     aingle_resolved_wunion_branch(self));
	}

	return 0;
}

static int
aingle_resolved_wunion_get_real_src(const aingle_value_iface_t *viface,
				  const void *vself, aingle_value_t *real_src)
{
	int  rval;
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_wunion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_wunion_reader_t, parent);
	aingle_resolved_wunion_value_t  *self = (aingle_resolved_wunion_value_t *) vself;
	int  writer_disc;
	check(rval, aingle_value_get_discriminant(&self->wrapped, &writer_disc));
	DEBUG("Writer is branch %d", writer_disc);

	if (uiface->branch_resolvers[writer_disc] == NULL) {
		aingle_set_error("Reader isn't compatible with writer branch %d",
			       writer_disc);
		return EINVAL;
	}

	if (self->discriminant == writer_disc) {
		DEBUG("Writer branch %d already selected", writer_disc);
	} else {
		if (self->discriminant >= 0) {
			DEBUG("Finalizing old writer branch %d", self->discriminant);
			aingle_resolved_reader_done
			    (uiface->branch_resolvers[self->discriminant],
			     aingle_resolved_wunion_branch(self));
		}
		DEBUG("Initializing writer branch %d", writer_disc);
		check(rval, aingle_resolved_reader_init
		      (uiface->branch_resolvers[writer_disc],
		       aingle_resolved_wunion_branch(self)));
		self->discriminant = writer_disc;
	}

	real_src->iface = &uiface->branch_resolvers[writer_disc]->parent;
	real_src->self = aingle_resolved_wunion_branch(self);
	return aingle_value_get_current_branch(&self->wrapped, (aingle_value_t *) real_src->self);
}

static int
aingle_resolved_wunion_reader_get_boolean(const aingle_value_iface_t *viface,
					const void *vself, int *out)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_boolean(&src, out);
}

static int
aingle_resolved_wunion_reader_get_bytes(const aingle_value_iface_t *viface,
				      const void *vself, const void **buf, size_t *size)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_bytes(&src, buf, size);
}

static int
aingle_resolved_wunion_reader_grab_bytes(const aingle_value_iface_t *viface,
				       const void *vself, aingle_wrapped_buffer_t *dest)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_grab_bytes(&src, dest);
}

static int
aingle_resolved_wunion_reader_get_double(const aingle_value_iface_t *viface,
				       const void *vself, double *out)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_double(&src, out);
}

static int
aingle_resolved_wunion_reader_get_float(const aingle_value_iface_t *viface,
				      const void *vself, float *out)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_float(&src, out);
}

static int
aingle_resolved_wunion_reader_get_int(const aingle_value_iface_t *viface,
				    const void *vself, int32_t *out)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_int(&src, out);
}

static int
aingle_resolved_wunion_reader_get_long(const aingle_value_iface_t *viface,
				     const void *vself, int64_t *out)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_long(&src, out);
}

static int
aingle_resolved_wunion_reader_get_null(const aingle_value_iface_t *viface,
				     const void *vself)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_null(&src);
}

static int
aingle_resolved_wunion_reader_get_string(const aingle_value_iface_t *viface,
				       const void *vself, const char **str, size_t *size)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_string(&src, str, size);
}

static int
aingle_resolved_wunion_reader_grab_string(const aingle_value_iface_t *viface,
					const void *vself, aingle_wrapped_buffer_t *dest)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_grab_string(&src, dest);
}

static int
aingle_resolved_wunion_reader_get_enum(const aingle_value_iface_t *viface,
				     const void *vself, int *out)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_enum(&src, out);
}

static int
aingle_resolved_wunion_reader_get_fixed(const aingle_value_iface_t *viface,
				      const void *vself, const void **buf, size_t *size)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_fixed(&src, buf, size);
}

static int
aingle_resolved_wunion_reader_grab_fixed(const aingle_value_iface_t *viface,
				       const void *vself, aingle_wrapped_buffer_t *dest)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_grab_fixed(&src, dest);
}

static int
aingle_resolved_wunion_reader_set_boolean(const aingle_value_iface_t *viface,
					void *vself, int val)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_boolean(&src, val);
}

static int
aingle_resolved_wunion_reader_set_bytes(const aingle_value_iface_t *viface,
				      void *vself, void *buf, size_t size)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_bytes(&src, buf, size);
}

static int
aingle_resolved_wunion_reader_give_bytes(const aingle_value_iface_t *viface,
				       void *vself, aingle_wrapped_buffer_t *buf)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_give_bytes(&src, buf);
}

static int
aingle_resolved_wunion_reader_set_double(const aingle_value_iface_t *viface,
				       void *vself, double val)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_double(&src, val);
}

static int
aingle_resolved_wunion_reader_set_float(const aingle_value_iface_t *viface,
				      void *vself, float val)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_float(&src, val);
}

static int
aingle_resolved_wunion_reader_set_int(const aingle_value_iface_t *viface,
				    void *vself, int32_t val)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_int(&src, val);
}

static int
aingle_resolved_wunion_reader_set_long(const aingle_value_iface_t *viface,
				     void *vself, int64_t val)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_long(&src, val);
}

static int
aingle_resolved_wunion_reader_set_null(const aingle_value_iface_t *viface,
				     void *vself)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_null(&src);
}

static int
aingle_resolved_wunion_reader_set_string(const aingle_value_iface_t *viface,
				       void *vself, const char *str)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_string(&src, str);
}

static int
aingle_resolved_wunion_reader_set_string_len(const aingle_value_iface_t *viface,
					   void *vself, const char *str, size_t size)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_string_len(&src, str, size);
}

static int
aingle_resolved_wunion_reader_give_string_len(const aingle_value_iface_t *viface,
					    void *vself, aingle_wrapped_buffer_t *buf)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_give_string_len(&src, buf);
}

static int
aingle_resolved_wunion_reader_set_enum(const aingle_value_iface_t *viface,
				     void *vself, int val)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_enum(&src, val);
}

static int
aingle_resolved_wunion_reader_set_fixed(const aingle_value_iface_t *viface,
				      void *vself, void *buf, size_t size)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_fixed(&src, buf, size);
}

static int
aingle_resolved_wunion_reader_give_fixed(const aingle_value_iface_t *viface,
				       void *vself, aingle_wrapped_buffer_t *dest)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_give_fixed(&src, dest);
}

static int
aingle_resolved_wunion_reader_get_size(const aingle_value_iface_t *viface,
				     const void *vself, size_t *size)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_size(&src, size);
}

static int
aingle_resolved_wunion_reader_get_by_index(const aingle_value_iface_t *viface,
					 const void *vself, size_t index,
					 aingle_value_t *child, const char **name)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_by_index(&src, index, child, name);
}

static int
aingle_resolved_wunion_reader_get_by_name(const aingle_value_iface_t *viface,
					const void *vself, const char *name,
					aingle_value_t *child, size_t *index)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_by_name(&src, name, child, index);
}

static int
aingle_resolved_wunion_reader_get_discriminant(const aingle_value_iface_t *viface,
					     const void *vself, int *out)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_discriminant(&src, out);
}

static int
aingle_resolved_wunion_reader_get_current_branch(const aingle_value_iface_t *viface,
					       const void *vself, aingle_value_t *branch)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_get_current_branch(&src, branch);
}

static int
aingle_resolved_wunion_reader_append(const aingle_value_iface_t *viface,
				   void *vself, aingle_value_t *child_out,
				   size_t *new_index)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_append(&src, child_out, new_index);
}

static int
aingle_resolved_wunion_reader_add(const aingle_value_iface_t *viface,
				void *vself, const char *key,
				aingle_value_t *child, size_t *index, int *is_new)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_add(&src, key, child, index, is_new);
}

static int
aingle_resolved_wunion_reader_set_branch(const aingle_value_iface_t *viface,
				       void *vself, int discriminant,
				       aingle_value_t *branch)
{
	int  rval;
	aingle_value_t  src;
	check(rval, aingle_resolved_wunion_get_real_src(viface, vself, &src));
	return aingle_value_set_branch(&src, discriminant, branch);
}

static aingle_resolved_wunion_reader_t *
aingle_resolved_wunion_reader_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_reader_t  *self = (aingle_resolved_reader_t *) aingle_new(aingle_resolved_wunion_reader_t);
	memset(self, 0, sizeof(aingle_resolved_wunion_reader_t));

	self->parent.incref_iface = aingle_resolved_reader_incref_iface;
	self->parent.decref_iface = aingle_resolved_reader_decref_iface;
	self->parent.incref = aingle_resolved_reader_incref;
	self->parent.decref = aingle_resolved_reader_decref;
	self->parent.reset = aingle_resolved_reader_reset;
	self->parent.get_type = aingle_resolved_reader_get_type;
	self->parent.get_schema = aingle_resolved_reader_get_schema;

	self->parent.get_boolean = aingle_resolved_wunion_reader_get_boolean;
	self->parent.grab_bytes = aingle_resolved_wunion_reader_grab_bytes;
	self->parent.get_bytes = aingle_resolved_wunion_reader_get_bytes;
	self->parent.get_double = aingle_resolved_wunion_reader_get_double;
	self->parent.get_float = aingle_resolved_wunion_reader_get_float;
	self->parent.get_int = aingle_resolved_wunion_reader_get_int;
	self->parent.get_long = aingle_resolved_wunion_reader_get_long;
	self->parent.get_null = aingle_resolved_wunion_reader_get_null;
	self->parent.get_string = aingle_resolved_wunion_reader_get_string;
	self->parent.grab_string = aingle_resolved_wunion_reader_grab_string;
	self->parent.get_enum = aingle_resolved_wunion_reader_get_enum;
	self->parent.get_fixed = aingle_resolved_wunion_reader_get_fixed;
	self->parent.grab_fixed = aingle_resolved_wunion_reader_grab_fixed;

	self->parent.set_boolean = aingle_resolved_wunion_reader_set_boolean;
	self->parent.set_bytes = aingle_resolved_wunion_reader_set_bytes;
	self->parent.give_bytes = aingle_resolved_wunion_reader_give_bytes;
	self->parent.set_double = aingle_resolved_wunion_reader_set_double;
	self->parent.set_float = aingle_resolved_wunion_reader_set_float;
	self->parent.set_int = aingle_resolved_wunion_reader_set_int;
	self->parent.set_long = aingle_resolved_wunion_reader_set_long;
	self->parent.set_null = aingle_resolved_wunion_reader_set_null;
	self->parent.set_string = aingle_resolved_wunion_reader_set_string;
	self->parent.set_string_len = aingle_resolved_wunion_reader_set_string_len;
	self->parent.give_string_len = aingle_resolved_wunion_reader_give_string_len;
	self->parent.set_enum = aingle_resolved_wunion_reader_set_enum;
	self->parent.set_fixed = aingle_resolved_wunion_reader_set_fixed;
	self->parent.give_fixed = aingle_resolved_wunion_reader_give_fixed;

	self->parent.get_size = aingle_resolved_wunion_reader_get_size;
	self->parent.get_by_index = aingle_resolved_wunion_reader_get_by_index;
	self->parent.get_by_name = aingle_resolved_wunion_reader_get_by_name;
	self->parent.get_discriminant = aingle_resolved_wunion_reader_get_discriminant;
	self->parent.get_current_branch = aingle_resolved_wunion_reader_get_current_branch;

	self->parent.append = aingle_resolved_wunion_reader_append;
	self->parent.add = aingle_resolved_wunion_reader_add;
	self->parent.set_branch = aingle_resolved_wunion_reader_set_branch;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->calculate_size = aingle_resolved_wunion_reader_calculate_size;
	self->free_iface = aingle_resolved_wunion_reader_free_iface;
	self->init = aingle_resolved_wunion_reader_init;
	self->done = aingle_resolved_wunion_reader_done;
	self->reset_wrappers = aingle_resolved_wunion_reader_reset;
	return container_of(self, aingle_resolved_wunion_reader_t, parent);
}

static aingle_resolved_reader_t *
try_writer_union(memoize_state_t *state,
		 aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * For a writer union, we check each branch of the union in turn
	 * against the reader schema.  For each one that is compatible,
	 * we save the child resolver that can be used to process a
	 * writer value of that branch.
	 */

	size_t  branch_count = aingle_schema_union_size(wschema);
	DEBUG("Checking %" PRIsz "-branch writer union schema", branch_count);

	aingle_resolved_wunion_reader_t  *uself =
	    aingle_resolved_wunion_reader_create(wschema, rschema);
	aingle_memoize_set(&state->mem, wschema, rschema, uself);

	aingle_resolved_reader_t  **branch_resolvers =
	    (aingle_resolved_reader_t **) aingle_calloc(branch_count, sizeof(aingle_resolved_reader_t *));
	int  some_branch_compatible = 0;

	size_t  i;
	for (i = 0; i < branch_count; i++) {
		aingle_schema_t  branch_schema =
		    aingle_schema_union_branch(wschema, i);

		DEBUG("Resolving writer union branch %" PRIsz " (%s)", i,
		      aingle_schema_type_name(branch_schema));

		/*
		 * Try to recursively resolve this branch of the writer
		 * union against the reader schema.  Don't raise
		 * an error if this fails — we just need one of
		 * the writer branches to be compatible.
		 */

		branch_resolvers[i] =
		    aingle_resolved_reader_new_memoized(state, branch_schema, rschema);
		if (branch_resolvers[i] == NULL) {
			DEBUG("No match for writer union branch %" PRIsz, i);
		} else {
			DEBUG("Found match for writer union branch %" PRIsz, i);
			some_branch_compatible = 1;
		}
	}

	/*
	 * If we didn't find a match, that's an error.
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

	aingle_free(branch_resolvers, branch_count * sizeof(aingle_resolved_reader_t *));
	return NULL;
}


/*-----------------------------------------------------------------------
 * reader union
 */

/*
 * For reader unions, we only resolve them against writers which aren't
 * unions.  (We'll have already broken any writer union apart into its
 * separate branches.)  We just have to record which branch of the
 * reader union the writer schema is compatible with.
 */

typedef struct aingle_resolved_runion_reader {
	aingle_resolved_reader_t  parent;

	/* The reader union branch that's compatible with the writer
	 * schema. */
	size_t  active_branch;

	/* A child resolver for the reader branch. */
	aingle_resolved_reader_t  *branch_resolver;
} aingle_resolved_runion_reader_t;


static void
aingle_resolved_runion_reader_calculate_size(aingle_resolved_reader_t *iface)
{
	aingle_resolved_runion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_runion_reader_t, parent);

	/* Only calculate the size for any resolver once */
	iface->calculate_size = NULL;

	DEBUG("Calculating size for %s->%s",
	      aingle_schema_type_name((iface)->wschema),
	      aingle_schema_type_name((iface)->rschema));

	aingle_resolved_reader_calculate_size(uiface->branch_resolver);
	iface->instance_size = uiface->branch_resolver->instance_size;
}


static void
aingle_resolved_runion_reader_free_iface(aingle_resolved_reader_t *iface, st_table *freeing)
{
	aingle_resolved_runion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_runion_reader_t, parent);

	if (uiface->branch_resolver != NULL) {
		free_resolver(uiface->branch_resolver, freeing);
	}

	aingle_schema_decref(iface->wschema);
	aingle_schema_decref(iface->rschema);
	aingle_freet(aingle_resolved_runion_reader_t, iface);
}

static int
aingle_resolved_runion_reader_init(const aingle_resolved_reader_t *iface, void *vself)
{
	aingle_resolved_runion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_runion_reader_t, parent);
	return aingle_resolved_reader_init(uiface->branch_resolver, vself);
}

static void
aingle_resolved_runion_reader_done(const aingle_resolved_reader_t *iface, void *vself)
{
	aingle_resolved_runion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_runion_reader_t, parent);
	aingle_resolved_reader_done(uiface->branch_resolver, vself);
}

static int
aingle_resolved_runion_reader_reset(const aingle_resolved_reader_t *iface, void *vself)
{
	aingle_resolved_runion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_runion_reader_t, parent);
	return aingle_resolved_reader_reset_wrappers(uiface->branch_resolver, vself);
}

static int
aingle_resolved_runion_reader_get_discriminant(const aingle_value_iface_t *viface,
					     const void *vself, int *out)
{
	AINGLE_UNUSED(vself);
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_runion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_runion_reader_t, parent);
	DEBUG("Reader union is branch %" PRIsz, uiface->active_branch);
	*out = uiface->active_branch;
	return 0;
}

static int
aingle_resolved_runion_reader_get_current_branch(const aingle_value_iface_t *viface,
					       const void *vself, aingle_value_t *branch)
{
	const aingle_resolved_reader_t  *iface =
	    container_of(viface, aingle_resolved_reader_t, parent);
	const aingle_resolved_runion_reader_t  *uiface =
	    container_of(iface, aingle_resolved_runion_reader_t, parent);
	DEBUG("Getting reader branch %" PRIsz " for union %p", uiface->active_branch, vself);
	branch->iface = &uiface->branch_resolver->parent;
	branch->self = (void *) vself;
	return 0;
}

static aingle_resolved_runion_reader_t *
aingle_resolved_runion_reader_create(aingle_schema_t wschema, aingle_schema_t rschema)
{
	aingle_resolved_reader_t  *self = (aingle_resolved_reader_t *) aingle_new(aingle_resolved_runion_reader_t);
	memset(self, 0, sizeof(aingle_resolved_runion_reader_t));

	self->parent.incref_iface = aingle_resolved_reader_incref_iface;
	self->parent.decref_iface = aingle_resolved_reader_decref_iface;
	self->parent.incref = aingle_resolved_reader_incref;
	self->parent.decref = aingle_resolved_reader_decref;
	self->parent.reset = aingle_resolved_reader_reset;
	self->parent.get_type = aingle_resolved_reader_get_type;
	self->parent.get_schema = aingle_resolved_reader_get_schema;
	self->parent.get_discriminant = aingle_resolved_runion_reader_get_discriminant;
	self->parent.get_current_branch = aingle_resolved_runion_reader_get_current_branch;

	self->refcount = 1;
	self->wschema = aingle_schema_incref(wschema);
	self->rschema = aingle_schema_incref(rschema);
	self->calculate_size = aingle_resolved_runion_reader_calculate_size;
	self->free_iface = aingle_resolved_runion_reader_free_iface;
	self->init = aingle_resolved_runion_reader_init;
	self->done = aingle_resolved_runion_reader_done;
	self->reset_wrappers = aingle_resolved_runion_reader_reset;
	return container_of(self, aingle_resolved_runion_reader_t, parent);
}

static aingle_resolved_reader_t *
try_reader_union(memoize_state_t *state,
		 aingle_schema_t wschema, aingle_schema_t rschema)
{
	/*
	 * For a reader union, we have to identify which branch
	 * corresponds to the writer schema.  (The writer won't be a
	 * union, since we'll have already broken it into its branches.)
	 */

	size_t  branch_count = aingle_schema_union_size(rschema);
	DEBUG("Checking %" PRIsz "-branch reader union schema", branch_count);

	aingle_resolved_runion_reader_t  *uself =
	    aingle_resolved_runion_reader_create(wschema, rschema);
	aingle_memoize_set(&state->mem, wschema, rschema, uself);

	size_t  i;
	for (i = 0; i < branch_count; i++) {
		aingle_schema_t  branch_schema =
		    aingle_schema_union_branch(rschema, i);

		DEBUG("Resolving reader union branch %" PRIsz " (%s)", i,
		      aingle_schema_type_name(branch_schema));

		/*
		 * Try to recursively resolve this branch of the reader
		 * union against the writer schema.  Don't raise
		 * an error if this fails — we just need one of
		 * the reader branches to be compatible.
		 */

		uself->branch_resolver =
		    aingle_resolved_reader_new_memoized(state, wschema, branch_schema);
		if (uself->branch_resolver == NULL) {
			DEBUG("No match for reader union branch %" PRIsz, i);
		} else {
			DEBUG("Found match for reader union branch %" PRIsz, i);
			uself->active_branch = i;
			return &uself->parent;
		}
	}

	/*
	 * If we didn't find a match, that's an error.
	 */

	DEBUG("No reader union branches match");
	aingle_set_error("No branches in the reader are compatible "
		       "with writer schema %s",
		       aingle_schema_type_name(wschema));
	goto error;

error:
	/*
	 * Clean up any resolver we might have already created.
	 */

	aingle_memoize_delete(&state->mem, wschema, rschema);
	aingle_value_iface_decref(&uself->parent.parent);
	return NULL;
}


/*-----------------------------------------------------------------------
 * Schema type dispatcher
 */

static aingle_resolved_reader_t *
aingle_resolved_reader_new_memoized(memoize_state_t *state,
				  aingle_schema_t wschema, aingle_schema_t rschema)
{
	check_param(NULL, is_aingle_schema(wschema), "writer schema");
	check_param(NULL, is_aingle_schema(rschema), "reader schema");

	/*
	 * First see if we've already matched these two schemas.  If so,
	 * just return that resolver.
	 */

	aingle_resolved_reader_t  *saved = NULL;
	if (aingle_memoize_get(&state->mem, wschema, rschema, (void **) &saved)) {
		DEBUG("Already resolved %s%s%s->%s%s%s",
		      is_aingle_link(wschema)? "[": "",
		      aingle_schema_type_name(wschema),
		      is_aingle_link(wschema)? "]": "",
		      is_aingle_link(rschema)? "[": "",
		      aingle_schema_type_name(rschema),
		      is_aingle_link(rschema)? "]": "");
		return saved;
	} else {
		DEBUG("Resolving %s%s%s->%s%s%s",
		      is_aingle_link(wschema)? "[": "",
		      aingle_schema_type_name(wschema),
		      is_aingle_link(wschema)? "]": "",
		      is_aingle_link(rschema)? "[": "",
		      aingle_schema_type_name(rschema),
		      is_aingle_link(rschema)? "]": "");
	}

	/*
	 * Otherwise we have some work to do.  First check if the writer
	 * schema is a union.  If so, break it apart.
	 */

	if (is_aingle_union(wschema)) {
		return try_writer_union(state, wschema, rschema);
	}

	else if (is_aingle_link(wschema)) {
		return try_wlink(state, wschema, rschema);
	}

	/*
	 * If the writer isn't a union, than choose a resolver based on
	 * the reader schema.
	 */

	switch (aingle_typeof(rschema))
	{
		case AINGLE_BOOLEAN:
			return try_boolean(state, wschema, rschema);

		case AINGLE_BYTES:
			return try_bytes(state, wschema, rschema);

		case AINGLE_DOUBLE:
			return try_double(state, wschema, rschema);

		case AINGLE_FLOAT:
			return try_float(state, wschema, rschema);

		case AINGLE_INT32:
			return try_int(state, wschema, rschema);

		case AINGLE_INT64:
			return try_long(state, wschema, rschema);

		case AINGLE_NULL:
			return try_null(state, wschema, rschema);

		case AINGLE_STRING:
			return try_string(state, wschema, rschema);

		case AINGLE_ARRAY:
			return try_array(state, wschema, rschema);

		case AINGLE_ENUM:
			return try_enum(state, wschema, rschema);

		case AINGLE_FIXED:
			return try_fixed(state, wschema, rschema);

		case AINGLE_MAP:
			return try_map(state, wschema, rschema);

		case AINGLE_RECORD:
			return try_record(state, wschema, rschema);

		case AINGLE_UNION:
			return try_reader_union(state, wschema, rschema);

		case AINGLE_LINK:
			return try_rlink(state, wschema, rschema);

		default:
			aingle_set_error("Unknown reader schema type");
			return NULL;
	}

	return NULL;
}


aingle_value_iface_t *
aingle_resolved_reader_new(aingle_schema_t wschema, aingle_schema_t rschema)
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

	aingle_resolved_reader_t  *result =
	    aingle_resolved_reader_new_memoized(&state, wschema, rschema);
	if (result == NULL) {
		aingle_memoize_done(&state.mem);
		return NULL;
	}

	/*
	 * Fix up any link schemas so that their value implementations
	 * point to their target schemas' implementations.
	 */

	aingle_resolved_reader_calculate_size(result);
	while (state.links != NULL) {
		aingle_resolved_link_reader_t  *liface = state.links;
		aingle_resolved_reader_calculate_size(liface->target_resolver);
		state.links = liface->next;
		liface->next = NULL;
	}

	/*
	 * And now we can return.
	 */

	aingle_memoize_done(&state.mem);
	return &result->parent;
}
