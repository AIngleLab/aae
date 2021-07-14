#!/bin/bash



set -e

cd `dirname "$0"`

dist_dir="../../dist/php"
build_dir="pkg"
version=$(cat ../../share/VERSION.txt)
libname="aingle-php-$version"
lib_dir="$build_dir/$libname"
tarball="$libname.tar.bz2"

test_tmp_dir="test/tmp"

function clean {
    rm -rf "$test_tmp_dir"
    rm -rf "$build_dir"
}

function dist {
    mkdir -p "$build_dir/$libname" "$lib_dir/examples"
    cp -pr lib "$lib_dir"
    cp -pr examples/*.php "$lib_dir/examples"
    cp README.md LICENSE NOTICE "$lib_dir"
    cd "$build_dir"
    tar -cjf "$tarball" "$libname"
    mkdir -p "../$dist_dir"
    cp "$tarball" "../$dist_dir"
}

for target in "$@"
do
  case "$target" in
    interop-data-generate)
      composer install -d "../.."
      php test/generate_interop_data.php
      ;;

    test-interop)
      composer install -d "../.."
      vendor/bin/phpunit test/InterOpTest.php
      ;;

    lint)
      composer install -d "../.."
      find . -name "*.php" -print0 | xargs -0 -n1 -P8 php -l
      vendor/bin/phpcs --standard=PSR12 lib
      ;;

    test)
      composer install -d "../.."
      vendor/bin/phpunit -v
      ;;

    dist)
      dist
      ;;

    clean)
      clean
      ;;

    *)
      echo "Usage: $0 {interop-data-generate|test-interop|lint|test|dist|clean}"
  esac
done

exit 0
