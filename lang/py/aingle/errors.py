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
# See the License for the specific language governing permissions and
# limitations under the License.

import json


def _safe_pretty(schema):
    """Try to pretty-print a schema, but never raise an exception within another exception."""
    try:
        return json.dumps(json.loads(str(schema)), indent=2)
    except Exception:  # Never raise an exception within another exception.
        return schema


class AIngleException(Exception):
    """The base class for exceptions in aingle."""


class SchemaParseException(AIngleException):
    """Raised when a schema failed to parse."""


class InvalidName(SchemaParseException):
    """User attempted to parse a schema with an invalid name."""


class AIngleWarning(UserWarning):
    """Base class for warnings."""


class IgnoredLogicalType(AIngleWarning):
    """Warnings for unknown or invalid logical types."""


class AIngleTypeException(AIngleException):
    """Raised when datum is not an example of schema."""

    def __init__(self, *args):
        try:
            expected_schema, name, datum = args[:3]
        except (IndexError, ValueError):
            return super().__init__(*args)
        pretty_expected = json.dumps(json.loads(str(expected_schema)), indent=2)
        return super().__init__(f'The datum "{datum}" provided for "{name}" is not an example of the schema {pretty_expected}')


class InvalidDefaultException(AIngleTypeException):
    """Raised when a default value isn't a suitable type for the schema."""


class AIngleOutOfScaleException(AIngleTypeException):
    """Raised when attempting to write a decimal datum with an exponent too large for the decimal schema."""

    def __init__(self, *args):
        try:
            scale, datum, exponent = args[:3]
        except (IndexError, ValueError):
            return super().__init__(*args)
        return super().__init__(f"The exponent of {datum}, {exponent}, is too large for the schema scale of {scale}")


class SchemaResolutionException(AIngleException):
    def __init__(self, fail_msg, writers_schema=None, readers_schema=None, *args):
        writers_message = f"\nWriter's Schema: {_safe_pretty(writers_schema)}" if writers_schema else ""
        readers_message = f"\nReader's Schema: {_safe_pretty(readers_schema)}" if readers_schema else ""
        super().__init__((fail_msg or "") + writers_message + readers_message, *args)


class DataFileException(AIngleException):
    """Raised when there's a problem reading or writing file object containers."""


class IONotReadyException(AIngleException):
    """Raised when attempting an aingle operation on an io object that isn't fully initialized."""


class AIngleRemoteException(AIngleException):
    """Raised when an error message is sent by an AIngle requestor or responder."""


class ConnectionClosedException(AIngleException):
    """Raised when attempting IPC on a closed connection."""


class ProtocolParseException(AIngleException):
    """Raised when a protocol failed to parse."""


class UnsupportedCodec(NotImplementedError, AIngleException):
    """Raised when the compression named cannot be used."""


class UsageError(RuntimeError, AIngleException):
    """An exception raised when incorrect arguments were passed."""


class AIngleRuntimeException(RuntimeError, AIngleException):
    """Raised when compatibility parsing encounters an unknown type"""
