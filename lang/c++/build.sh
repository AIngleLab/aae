#!/bin/bash



set -e # exit on error

function usage {
  echo "Usage: $0 {lint|test|dist|clean|install|doc|format}"
  exit 1
}

if [ $# -eq 0 ]
then
  usage
fi

if [ -f VERSION.txt ]
then
  VERSION=`cat VERSION.txt`
else
  VERSION=`cat ../../share/VERSION.txt`
fi

BUILD=../../build
AINGLE_CPP=aingle-cpp-$VERSION
AINGLE_DOC=aingle-doc-$VERSION
BUILD_DIR=../../build
BUILD_CPP=$BUILD/$AINGLE_CPP
DIST_DIR=../../dist/$AINGLE_CPP
DOC_CPP=$BUILD/$AINGLE_DOC/api/cpp
DIST_DIR=../../dist/cpp
TARFILE=../dist/cpp/$AINGLE_CPP.tar.gz

function do_doc() {
  doxygen
  if [ -d doc ]
  then
    mkdir -p $DOC_CPP
    cp -R doc/* $DOC_CPP
  else
    exit 1
  fi
}

function do_dist() {
  rm -rf $BUILD_CPP/
  mkdir -p $BUILD_CPP
  cp -r api AUTHORS build.sh CMakeLists.txt ChangeLog \
    LICENSE NOTICE impl jsonschemas NEWS parser README scripts test examples \
    $BUILD_CPP
  find $BUILD_CPP -name '.svn' | xargs rm -rf
  cp ../../share/VERSION.txt $BUILD_CPP
  mkdir -p $DIST_DIR
  (cd $BUILD_DIR; tar cvzf $TARFILE $AINGLE_CPP && cp $TARFILE $AINGLE_CPP )
  if [ ! -f $DIST_FILE ]
  then
    exit 1
  fi
}

(mkdir -p build; cd build; cmake -G "Unix Makefiles" ..)
for target in "$@"
do

case "$target" in
  lint)
    # some versions of cppcheck seem to require an explicit
    # "--error-exitcode" option to return non-zero code
    cppcheck --error-exitcode=1 --inline-suppr -f -q -x c++ .
    ;;

  test)
    (cd build && cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug -D AINGLE_ADD_PROTECTOR_FLAGS=1 .. && make && cd .. \
      && ./build/buffertest \
      && ./build/unittest \
      && ./build/CodecTests \
      && ./build/CompilerTests \
      && ./build/StreamTests \
      && ./build/SpecificTests \
      && ./build/AInglegencppTests \
      && ./build/DataFileTests   \
      && ./build/SchemaTests)
    ;;

  xcode-test)
    mkdir -p build.xcode
    (cd build.xcode \
        && cmake -G Xcode .. \
        && xcodebuild -configuration Release \
        && ctest -C Release)
    ;;

  dist)
    (cd build && cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release ..)
    do_dist
    do_doc
    ;;

  doc)
    do_doc
    ;;

  format)
    clang-format -i --style file `find api -type f` `find impl -type f` `find test -type f`
    ;;

  clean)
    (cd build && make clean)
    rm -rf doc test.aingle test?.df test??.df test_skip.df test_lastSync.df test_readRecordUsingLastSync.df
    ;;

  install)
    (cd build && cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release .. && make install)
    ;;

  *)
    usage
esac

done

exit 0
