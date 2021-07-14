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

#ifndef AINGLE_GENERIC_H
#define AINGLE_GENERIC_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <aingle/platform.h>
#include <stdlib.h>

#include <aingle/schema.h>
#include <aingle/value.h>

/*
 * This file contains an aingle_value_t implementation that can store
 * values of any AIngle schema.  It replaces the old aingle_datum_t class.
 */


/**
 * Return a generic aingle_value_iface_t implementation for the given
 * schema, regardless of what type it is.
 */

aingle_value_iface_t *
aingle_generic_class_from_schema(aingle_schema_t schema);

/**
 * Allocate a new instance of the given generic value class.  @a iface
 * must have been created by @ref aingle_generic_class_from_schema.
 */

int
aingle_generic_value_new(aingle_value_iface_t *iface, aingle_value_t *dest);


/*
 * These functions return an aingle_value_iface_t implementation for each
 * primitive schema type.  (For enum, fixed, and the compound types, you
 * must use the @ref aingle_generic_class_from_schema function.)
 */

aingle_value_iface_t *aingle_generic_boolean_class(void);
aingle_value_iface_t *aingle_generic_bytes_class(void);
aingle_value_iface_t *aingle_generic_double_class(void);
aingle_value_iface_t *aingle_generic_float_class(void);
aingle_value_iface_t *aingle_generic_int_class(void);
aingle_value_iface_t *aingle_generic_long_class(void);
aingle_value_iface_t *aingle_generic_null_class(void);
aingle_value_iface_t *aingle_generic_string_class(void);


/*
 * These functions instantiate a new generic primitive value.
 */

int aingle_generic_boolean_new(aingle_value_t *value, int val);
int aingle_generic_bytes_new(aingle_value_t *value, void *buf, size_t size);
int aingle_generic_double_new(aingle_value_t *value, double val);
int aingle_generic_float_new(aingle_value_t *value, float val);
int aingle_generic_int_new(aingle_value_t *value, int32_t val);
int aingle_generic_long_new(aingle_value_t *value, int64_t val);
int aingle_generic_null_new(aingle_value_t *value);
int aingle_generic_string_new(aingle_value_t *value, const char *val);
int aingle_generic_string_new_length(aingle_value_t *value, const char *val, size_t size);


CLOSE_EXTERN
#endif
