# frozen_string_literal: true
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

require 'multi_json'
require 'set'
require 'digest/md5'
require 'net/http'
require 'stringio'
require 'zlib'

module AIngle
  VERSION = File.read("#{__dir__}/aingle/VERSION.txt").freeze

  class AIngleError < StandardError; end

  class AIngleTypeError < AIngle::AIngleError
    def initialize(schm=nil, datum=nil, msg=nil)
      msg ||= "Not a #{schm}: #{datum}"
      super(msg)
    end
  end

  class << self
    attr_writer :disable_enum_symbol_validation
    attr_writer :disable_field_default_validation
    attr_writer :disable_schema_name_validation

    def disable_enum_symbol_validation
      @disable_enum_symbol_validation ||=
        ENV.fetch('AINGLE_DISABLE_ENUM_SYMBOL_VALIDATION', '') != ''
    end

    def disable_field_default_validation
      @disable_field_default_validation ||=
        ENV.fetch('AINGLE_DISABLE_FIELD_DEFAULT_VALIDATION', '') != ''
    end

    def disable_schema_name_validation
      @disable_schema_name_validation ||=
        ENV.fetch('AINGLE_DISABLE_SCHEMA_NAME_VALIDATION', '') != ''
    end
  end
end

require 'aingle/schema'
require 'aingle/io'
require 'aingle/data_file'
require 'aingle/protocol'
require 'aingle/ipc'
require 'aingle/schema_normalization'
require 'aingle/schema_validator'
require 'aingle/schema_compatibility'
