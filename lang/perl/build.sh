#!/bin/bash



set -e # exit on error

function usage {
  echo "Usage: $0 {lint|test|dist|clean|interop-data-generate|interop-data-test}"
  exit 1
}

if [ $# -eq 0 ]
then
  usage
fi

for target in "$@"
do

function do_clean(){
  [ ! -f Makefile ] || make clean
  rm -f  AIngle-*.tar.gz META.yml Makefile.old
  rm -rf lang/perl/inc/
}

function do_lint(){
  local failures=0
  for i in $(find lib t xt -name '*.p[lm]' -or -name '*.t'); do
    if ! perlcritic --verbose 1 ${i}; then
      ((failures=failures+1))
    fi
  done
  if [ ${failures} -gt 0 ]; then
    return 1
  fi
}

case "$target" in
  lint)
    do_lint
    ;;

  test)
    perl ./Makefile.PL && make test
    ;;

  dist)
    cp ../../share/VERSION.txt .
    perl ./Makefile.PL && make dist
    ;;

  clean)
    do_clean
    ;;

  interop-data-generate)
    perl -Ilib share/interop-data-generate
    ;;

  interop-data-test)
    prove -Ilib xt/interop.t
    ;;

  *)
    usage
esac

done

exit 0
