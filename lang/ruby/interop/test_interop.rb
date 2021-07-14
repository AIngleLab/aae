#!/usr/bin/env ruby
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

require 'rubygems'
require 'test/unit'
require 'aingle'

CODECS_TO_VALIDATE = ['deflate', 'snappy', 'zstandard'].freeze  # The 'null' codec is implicitly included

class TestInterop < Test::Unit::TestCase
  HERE = File.expand_path(File.dirname(__FILE__))
  SHARE = HERE + '/../../../share'
  SCHEMAS = SHARE + '/test/schemas'

  files = Dir[HERE + '/../../../build/interop/data/*.aingle'].select do |fn|
    sep, codec = File.basename(fn, '.aingle').rpartition('_')[1, 2]
    sep.empty? || CODECS_TO_VALIDATE.include?(codec)
  end
  puts "The following files will be tested:"
  puts files

  files.each do |fn|
    define_method("test_read_#{File.basename(fn, '.aingle')}") do
      projection = AIngle::Schema.parse(File.read(SCHEMAS+'/interop.ain'))

      File.open(fn) do |f|
        r = AIngle::DataFile::Reader.new(f, AIngle::IO::DatumReader.new(projection))
        i = 0
        r.each do |datum|
          i += 1
          assert_not_nil datum, "nil datum from #{fn}"
        end
        assert_not_equal 0, i, "no data read in from #{fn}"
      end
    end
  end
end
