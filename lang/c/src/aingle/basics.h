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

#ifndef AINGLE_BASICS_H
#define AINGLE_BASICS_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif


enum aingle_type_t {
	AINGLE_STRING,
	AINGLE_BYTES,
	AINGLE_INT32,
	AINGLE_INT64,
	AINGLE_FLOAT,
	AINGLE_DOUBLE,
	AINGLE_BOOLEAN,
	AINGLE_NULL,
	AINGLE_RECORD,
	AINGLE_ENUM,
	AINGLE_FIXED,
	AINGLE_MAP,
	AINGLE_ARRAY,
	AINGLE_UNION,
	AINGLE_LINK
};
typedef enum aingle_type_t aingle_type_t;

enum aingle_class_t {
	AINGLE_SCHEMA,
	AINGLE_DATUM
};
typedef enum aingle_class_t aingle_class_t;

struct aingle_obj_t {
	aingle_type_t type;
	aingle_class_t class_type;
	volatile int  refcount;
};

#define aingle_classof(obj)     ((obj)->class_type)
#define is_aingle_schema(obj)   (obj && aingle_classof(obj) == AINGLE_SCHEMA)
#define is_aingle_datum(obj)    (obj && aingle_classof(obj) == AINGLE_DATUM)

#define aingle_typeof(obj)      ((obj)->type)
#define is_aingle_string(obj)   (obj && aingle_typeof(obj) == AINGLE_STRING)
#define is_aingle_bytes(obj)    (obj && aingle_typeof(obj) == AINGLE_BYTES)
#define is_aingle_int32(obj)    (obj && aingle_typeof(obj) == AINGLE_INT32)
#define is_aingle_int64(obj)    (obj && aingle_typeof(obj) == AINGLE_INT64)
#define is_aingle_float(obj)    (obj && aingle_typeof(obj) == AINGLE_FLOAT)
#define is_aingle_double(obj)   (obj && aingle_typeof(obj) == AINGLE_DOUBLE)
#define is_aingle_boolean(obj)  (obj && aingle_typeof(obj) == AINGLE_BOOLEAN)
#define is_aingle_null(obj)     (obj && aingle_typeof(obj) == AINGLE_NULL)
#define is_aingle_primitive(obj)(is_aingle_string(obj) \
                             ||is_aingle_bytes(obj) \
                             ||is_aingle_int32(obj) \
                             ||is_aingle_int64(obj) \
                             ||is_aingle_float(obj) \
                             ||is_aingle_double(obj) \
                             ||is_aingle_boolean(obj) \
                             ||is_aingle_null(obj))
#define is_aingle_record(obj)   (obj && aingle_typeof(obj) == AINGLE_RECORD)
#define is_aingle_enum(obj)     (obj && aingle_typeof(obj) == AINGLE_ENUM)
#define is_aingle_fixed(obj)    (obj && aingle_typeof(obj) == AINGLE_FIXED)
#define is_aingle_named_type(obj)(is_aingle_record(obj) \
                              ||is_aingle_enum(obj) \
                              ||is_aingle_fixed(obj))
#define is_aingle_map(obj)      (obj && aingle_typeof(obj) == AINGLE_MAP)
#define is_aingle_array(obj)    (obj && aingle_typeof(obj) == AINGLE_ARRAY)
#define is_aingle_union(obj)    (obj && aingle_typeof(obj) == AINGLE_UNION)
#define is_aingle_complex_type(obj) (!(is_aingle_primitive(obj))
#define is_aingle_link(obj)     (obj && aingle_typeof(obj) == AINGLE_LINK)



CLOSE_EXTERN
#endif
