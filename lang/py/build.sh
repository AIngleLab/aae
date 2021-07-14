#!/bin/bash



set -e

usage() {
  echo "Usage: $0 {clean|dist|interop-data-generate|interop-data-test|lint|test}"
  exit 1
}

clean() {
  git clean -xdf '*.avpr' \
                 '*.ain' \
                 '*.egg-info' \
                 '*.py[co]' \
                 'VERSION.txt' \
                 '__pycache__' \
                 '.tox' \
                 'aingle/test/interop' \
                 'dist' \
                 'userlogs'
}

dist() {
  python3 setup.py sdist
  python3 setup.py bdist_wheel
  mkdir -p ../../dist/py
  cp dist/*.{tar.gz,whl} ../../dist/py
}

interop-data-generate() {
  ./setup.py generate_interop_data
  cp -r aingle/test/interop/data ../../build/interop
}

interop-data-test() {
  mkdir -p aingle/test/interop ../../build/interop/data
  cp -r ../../build/interop/data aingle/test/interop
  python3 -m unittest aingle.test.test_datafile_interop
}

lint() {
  python3 -m tox -e lint
}

test_() {
  TOX_SKIP_ENV=lint python3 -m tox --skip-missing-interpreters
}

main() {
  (( $# )) || usage
  for target; do
    case "$target" in
      clean) clean;;
      dist) dist;;
      interop-data-generate) interop-data-generate;;
      interop-data-test) interop-data-test;;
      lint) lint;;
      test) test_;;
      *) usage;;
    esac
  done
}

main "$@"
