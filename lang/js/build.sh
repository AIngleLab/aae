#!/bin/bash



set -e

cd `dirname "$0"`

for target in "$@"
do
  case "$target" in
    lint)
      npm install
      npm run lint
      ;;
    test)
      npm install
      npm run cover
      ;;
    dist)
      npm pack
      mkdir -p ../../dist/js
      mv aingle-js-*.tgz ../../dist/js
      ;;
    clean)
      rm -rf coverage
      ;;
    interop-data-generate)
      npm run interop-data-generate
      ;;
    interop-data-test)
      npm run interop-data-test
      ;;
    *)
      echo "Usage: $0 {lint|test|dist|clean}" >&2
      exit 1
  esac
done
