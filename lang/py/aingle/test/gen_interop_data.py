#!/usr/bin/env python3

##
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#     
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys

import aingle.codecs
import aingle.datafile
import aingle.io
import aingle.schema

NULL_CODEC = "null"
CODECS_TO_VALIDATE = aingle.codecs.KNOWN_CODECS.keys()

DATUM = {
    "intField": 12,
    "longField": 15234324,
    "stringField": "hey",
    "boolField": True,
    "floatField": 1234.0,
    "doubleField": -1234.0,
    "bytesField": b"12312adf",
    "nullField": None,
    "arrayField": [5.0, 0.0, 12.0],
    "mapField": {"a": {"label": "a"}, "bee": {"label": "cee"}},
    "unionField": 12.0,
    "enumField": "C",
    "fixedField": b"1019181716151413",
    "recordField": {"label": "blah", "children": [{"label": "inner", "children": []}]},
}


def generate(schema_path, output_path):
    with open(schema_path) as schema_file:
        interop_schema = aingle.schema.parse(schema_file.read())
    for codec in CODECS_TO_VALIDATE:
        filename = output_path
        if codec != NULL_CODEC:
            base, ext = os.path.splitext(output_path)
            filename = base + "_" + codec + ext
        with aingle.datafile.DataFileWriter(open(filename, "wb"), aingle.io.DatumWriter(), interop_schema, codec=codec) as dfw:
            dfw.append(DATUM)


if __name__ == "__main__":
    generate(sys.argv[1], sys.argv[2])
