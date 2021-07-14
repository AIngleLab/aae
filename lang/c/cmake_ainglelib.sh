#!/bin/bash
#


mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=ainglelib -DCMAKE_BUILD_TYPE=Debug
make
make test
make install
mkdir -p ainglelib/lib/static
cp ainglelib/lib/libaingle.a ainglelib/lib/static/libaingle.a
