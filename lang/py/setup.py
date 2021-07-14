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


import distutils.errors
import glob
import os
import subprocess

import setuptools  # type: ignore

_HERE = os.path.dirname(os.path.abspath(__file__))
_AINGLE_DIR = os.path.join(_HERE, "aingle")
_VERSION_FILE_NAME = "VERSION.txt"


def _is_distribution():
    """Tests whether setup.py is invoked from a distribution.

    Returns:
        True if setup.py runs from a distribution.
        False otherwise, ie. if setup.py runs from a version control work tree.
    """
    # If a file PKG-INFO exists as a sibling of setup.py,
    # assume we are running as source distribution:
    return os.path.exists(os.path.join(_HERE, "PKG-INFO"))


def _generate_package_data():
    """Generate package data.

    This data will already exist in a distribution package,
    so this function only runs for local version control work tree.
    """
    distutils.log.info("Generating package data")

    # AIngle top-level source directory:
    root_dir = os.path.dirname(os.path.dirname(_HERE))
    share_dir = os.path.join(root_dir, "share")

    # Create a PEP440 compliant version file.
    version_file_path = os.path.join(share_dir, _VERSION_FILE_NAME)
    with open(version_file_path, "rb") as vin:
        version = vin.read().replace(b"-", b"+")
    with open(os.path.join(_AINGLE_DIR, _VERSION_FILE_NAME), "wb") as vout:
        vout.write(version)

    aingle_schemas_dir = os.path.join(share_dir, "schemas", "org", "apache", "aingle")
    ipc_dir = os.path.join(aingle_schemas_dir, "ipc")
    tether_dir = os.path.join(aingle_schemas_dir, "mapred", "tether")

    # Copy necessary ain files:
    ain_files = (
        ((share_dir, "test", "schemas", "interop.ain"), ("",)),
        ((ipc_dir, "HandshakeRequest.ain"), ("",)),
        ((ipc_dir, "HandshakeResponse.ain"), ("",)),
        ((tether_dir, "InputProtocol.avpr"), ("tether",)),
        ((tether_dir, "OutputProtocol.avpr"), ("tether",)),
    )

    for src, dst in ain_files:
        src = os.path.join(*src)
        dst = os.path.join(_AINGLE_DIR, *dst)
        distutils.file_util.copy_file(src, dst)


class GenerateInteropDataCommand(setuptools.Command):
    """A command to generate AIngle files for data interop test."""

    user_options = [
        ("schema-file=", None, "path to input AIngle schema file"),
        ("output-path=", None, "path to output AIngle data files"),
    ]

    def initialize_options(self):
        self.schema_file = os.path.join(_AINGLE_DIR, "interop.ain")
        self.output_path = os.path.join(_AINGLE_DIR, "test", "interop", "data")

    def finalize_options(self):
        pass

    def run(self):
        # Late import -- this can only be run when aingle is on the pythonpath,
        # more or less after install.
        import aingle.test.gen_interop_data

        if not os.path.exists(self.output_path):
            os.makedirs(self.output_path)
        aingle.test.gen_interop_data.generate(self.schema_file, os.path.join(self.output_path, "py.aingle"))


def _get_version():
    curdir = os.getcwd()
    if os.path.isfile("aingle/VERSION.txt"):
        version_file = "aingle/VERSION.txt"
    else:
        index = curdir.index("lang/py")
        path = curdir[:index]
        version_file = os.path.join(path, "share/VERSION.txt")
    with open(version_file) as verfile:
        # To follow the naming convention defined by PEP 440
        # in the case that the version is like "x.y.z-SNAPSHOT"
        return verfile.read().rstrip().replace("-", "+")


def main():
    if not _is_distribution():
        _generate_package_data()

    setuptools.setup(
        cmdclass={
            "generate_interop_data": GenerateInteropDataCommand,
        }
    )


if __name__ == "__main__":
    main()
