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

import collections
import distutils.spawn
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import unittest

import aingle
import aingle.datafile
import aingle.io
import aingle.schema
import aingle.tether.tether_task_runner

_AINGLE_DIR = os.path.abspath(os.path.dirname(aingle.__file__))


def _version():
    with open(os.path.join(_AINGLE_DIR, "VERSION.txt")) as v:
        # Convert it back to the java version
        return v.read().strip().replace("+", "-")


_AINGLE_VERSION = _version()
_JAR_PATH = os.path.join(
    os.path.dirname(os.path.dirname(_AINGLE_DIR)),
    "java",
    "tools",
    "target",
    f"aingle-tools-{_AINGLE_VERSION}.jar",
)

_LINES = (
    "the quick brown fox jumps over the lazy dog",
    "the cow jumps over the moon",
    "the rain in spain falls mainly on the plains",
)
_IN_SCHEMA = '"string"'

# The schema for the output of the mapper and reducer
_OUT_SCHEMA = """{
  "type": "record",
  "name": "Pair",
  "namespace": "org.apache.aingle.mapred",
  "fields": [{"name": "key", "type": "string"},
             {"name": "value", "type": "long", "order": "ignore"}]
}"""

_PYTHON_PATH = os.pathsep.join([os.path.dirname(os.path.dirname(aingle.__file__)), os.path.dirname(__file__)])


def _has_java():  # pragma: no coverage
    """Detect if this system has a usable java installed.

    On most systems, this is just checking if `java` is in the PATH.

    But macos always has a /usr/bin/java, which does not mean java is installed.
    If you invoke java on macos and java is not installed, macos will spawn a popup
    telling you how to install java. This code does additional work around that
    to be completely automatic.
    """
    if platform.system() == "Darwin":
        try:
            output = subprocess.check_output("/usr/libexec/java_home", stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            output = e.output
        return b"No Java runtime present" not in output
    return bool(distutils.spawn.find_executable("java"))


@unittest.skipUnless(_has_java(), "No Java runtime present")
@unittest.skipUnless(os.path.exists(_JAR_PATH), f"{_JAR_PATH} not found")
class TestTetherWordCount(unittest.TestCase):
    """unittest for a python tethered map-reduce job."""

    _base_dir = None
    _script_path = None
    _input_path = None
    _output_path = None
    _output_schema_path = None

    def setUp(self):
        """Create temporary files for testing."""
        prefix, _ = os.path.splitext(os.path.basename(__file__))
        self._base_dir = tempfile.mkdtemp(prefix=prefix)

        # We create the input path...
        self._input_path = os.path.join(self._base_dir, "in")
        if not os.path.exists(self._input_path):
            os.makedirs(self._input_path)
        infile = os.path.join(self._input_path, "lines.aingle")
        self._write_lines(_LINES, infile)
        self.assertTrue(os.path.exists(infile), f"Missing the input file {infile}")

        # ...and the output schema...
        self._output_schema_path = os.path.join(self._base_dir, "output.ain")
        with open(self._output_schema_path, "w") as output_schema_handle:
            output_schema_handle.write(_OUT_SCHEMA)
        self.assertTrue(os.path.exists(self._output_schema_path), "Missing the schema file")

        # ...but we just name the output path. The tether tool creates it.
        self._output_path = os.path.join(self._base_dir, "out")

    def tearDown(self):
        """Remove temporary files used in testing."""
        if os.path.exists(self._base_dir):
            shutil.rmtree(self._base_dir)
        if self._script_path is not None and os.path.exists(self._script_path):
            os.remove(self._script_path)

    def _write_lines(self, lines, fname):
        """
        Write the lines to an aingle file named fname

        Parameters
        --------------------------------------------------------
        lines - list of strings to write
        fname - the name of the file to write to.
        """
        datum_writer = aingle.io.DatumWriter(_IN_SCHEMA)
        writers_schema = aingle.schema.parse(_IN_SCHEMA)
        with aingle.datafile.DataFileWriter(open(fname, "wb"), datum_writer, writers_schema) as writer:
            for datum in lines:
                writer.append(datum)

    def test_tether_word_count(self):
        """Check that a tethered map-reduce job produces the output expected locally."""
        # Run the job...
        args = (
            "java",
            "-jar",
            _JAR_PATH,
            "tether",
            "--protocol",
            "http",
            "--in",
            self._input_path,
            "--out",
            self._output_path,
            "--outschema",
            self._output_schema_path,
            "--program",
            sys.executable,
            "--exec_args",
            "-m aingle.tether.tether_task_runner word_count_task.WordCountTask",
        )
        print(f"Command:\n\t{' '.join(args)}")
        subprocess.check_call(args, env={"PYTHONPATH": _PYTHON_PATH, "PATH": os.environ["PATH"]})

        # ...and test the results.
        datum_reader = aingle.io.DatumReader()
        outfile = os.path.join(self._output_path, "part-00000.aingle")
        expected_counts = collections.Counter(" ".join(_LINES).split())
        with aingle.datafile.DataFileReader(open(outfile, "rb"), datum_reader) as reader:
            actual_counts = {r["key"]: r["value"] for r in reader}
        self.assertDictEqual(actual_counts, expected_counts)


if __name__ == "__main__":  # pragma: no coverage
    unittest.main()
