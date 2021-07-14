%{
/*
 Licensed to the Apache Software Foundation (ASF) under one
 or more contributor license agreements.  See the NOTICE file
 distributed with this work for additional information
 regarding copyright ownership.  The ASF licenses this file
 to you under the Apache License, Version 2.0 (the
 "License"); you may not use this file except in compliance
 with the License.  You may obtain a copy of the License at
 
 
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include <boost/format.hpp>
#include "Compiler.hh"
#include "Exception.hh"

#define YYLEX_PARAM ctx
#define YYPARSE_PARAM ctx

void yyerror(const char *str)
{
    throw aingle::Exception(boost::format("Parser error: %1%") % str);
}

extern void *lexer;
extern int yylex(int *, void *);

aingle::CompilerContext &context(void *ctx) {
    return *static_cast<aingle::CompilerContext *>(ctx);
};

%}

%pure-parser
%error-verbose

%token AINGLE_LEX_INT AINGLE_LEX_LONG
%token AINGLE_LEX_FLOAT AINGLE_LEX_DOUBLE
%token AINGLE_LEX_BOOL AINGLE_LEX_NULL
%token AINGLE_LEX_BYTES AINGLE_LEX_STRING
%token AINGLE_LEX_RECORD AINGLE_LEX_ENUM AINGLE_LEX_ARRAY AINGLE_LEX_MAP AINGLE_LEX_UNION AINGLE_LEX_FIXED

%token AINGLE_LEX_METADATA

%token AINGLE_LEX_SYMBOLS AINGLE_LEX_SYMBOLS_END
%token AINGLE_LEX_FIELDS AINGLE_LEX_FIELDS_END AINGLE_LEX_FIELD AINGLE_LEX_FIELD_END

%token AINGLE_LEX_TYPE AINGLE_LEX_ITEMS AINGLE_LEX_VALUES

// Tokens that output text:
%token AINGLE_LEX_OUTPUT_TEXT_BEGIN
%token AINGLE_LEX_NAME
%token AINGLE_LEX_NAMED_TYPE
%token AINGLE_LEX_FIELD_NAME
%token AINGLE_LEX_SYMBOL
%token AINGLE_LEX_SIZE
%token AINGLE_LEX_OUTPUT_TEXT_END

%token AINGLE_LEX_SIMPLE_TYPE

%%

aingleschema:
        simpleprimitive | object | union_t
        ;

primitive:
        AINGLE_LEX_INT    { context(ctx).addType(aingle::AINGLE_INT); }
        |
        AINGLE_LEX_LONG   { context(ctx).addType(aingle::AINGLE_LONG); }
        |
        AINGLE_LEX_FLOAT  { context(ctx).addType(aingle::AINGLE_FLOAT); }
        |
        AINGLE_LEX_DOUBLE { context(ctx).addType(aingle::AINGLE_DOUBLE); }
        |
        AINGLE_LEX_BOOL   { context(ctx).addType(aingle::AINGLE_BOOL); }
        |
        AINGLE_LEX_NULL   { context(ctx).addType(aingle::AINGLE_NULL); }
        |
        AINGLE_LEX_BYTES  { context(ctx).addType(aingle::AINGLE_BYTES); }
        |
        AINGLE_LEX_STRING { context(ctx).addType(aingle::AINGLE_STRING); }
        |
        AINGLE_LEX_NAMED_TYPE { context(ctx).addNamedType(); }
        ;

simpleprimitive:
        AINGLE_LEX_SIMPLE_TYPE { context(ctx).startType(); } primitive { context(ctx).stopType(); }
        ;

primitive_t:
        AINGLE_LEX_TYPE primitive
        ;

array_t:
        AINGLE_LEX_TYPE AINGLE_LEX_ARRAY { context(ctx).addType(aingle::AINGLE_ARRAY); }
        ;

enum_t:
        AINGLE_LEX_TYPE AINGLE_LEX_ENUM { context(ctx).addType(aingle::AINGLE_ENUM); }
        ;

fixed_t:
        AINGLE_LEX_TYPE AINGLE_LEX_FIXED { context(ctx).addType(aingle::AINGLE_FIXED); }
        ;

map_t:
        AINGLE_LEX_TYPE AINGLE_LEX_MAP { context(ctx).addType(aingle::AINGLE_MAP); }
        ;

record_t:
        AINGLE_LEX_TYPE AINGLE_LEX_RECORD { context(ctx).addType(aingle::AINGLE_RECORD); }
        ;

type_attribute:
        array_t | enum_t | fixed_t | map_t | record_t | primitive_t
        ;

union_t:
        '[' { context(ctx).startType(); context(ctx).addType(aingle::AINGLE_UNION); context(ctx).setTypesAttribute(); }
        unionlist
        ']' { context(ctx).stopType(); }
        ;

object:
        '{' { context(ctx).startType(); }
         attributelist
        '}' { context(ctx).stopType(); }
        ;

name_attribute:
        AINGLE_LEX_NAME { context(ctx).setNameAttribute(); }
        ;

size_attribute:
        AINGLE_LEX_SIZE { context(ctx).setSizeAttribute(); }
        ;

values_attribute:
        AINGLE_LEX_VALUES { context(ctx).setValuesAttribute(); } aingleschema
        ;

fields_attribute:
        AINGLE_LEX_FIELDS { context(ctx).setFieldsAttribute(); } fieldslist AINGLE_LEX_FIELDS_END
        ;

items_attribute:
        AINGLE_LEX_ITEMS { context(ctx).setItemsAttribute(); } aingleschema
        ;

symbols_attribute:
        AINGLE_LEX_SYMBOLS symbollist AINGLE_LEX_SYMBOLS_END
        ;

attribute:
        type_attribute | name_attribute | fields_attribute | items_attribute | size_attribute | values_attribute | symbols_attribute | AINGLE_LEX_METADATA
        ;

attributelist:
        attribute | attributelist ',' attribute
        ;

symbol:
        AINGLE_LEX_SYMBOL { context(ctx).setSymbolsAttribute(); }
        ;

symbollist:
        symbol | symbollist ',' symbol
        ;

fieldsetting:
        fieldname | aingleschema | AINGLE_LEX_METADATA
        ;

fieldsettinglist:
        fieldsetting | fieldsettinglist ',' fieldsetting
        ;

fields:
        AINGLE_LEX_FIELD fieldsettinglist AINGLE_LEX_FIELD_END
        ;

fieldname:
        AINGLE_LEX_FIELD_NAME { context(ctx).textContainsFieldName(); }
        ;

fieldslist:
        fields | fieldslist ',' fields
        ;

unionlist:
        aingleschema | unionlist ',' aingleschema
        ;
