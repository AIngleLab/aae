#!/bin/bash



set -e                # exit on error
set -x

cd `dirname "$0"`                  # connect to root

ROOT=../..
VERSION=`cat $ROOT/share/VERSION.txt`

for target in "$@"
do

  case "$target" in

    lint)
      echo 'This is a stub where someone can provide linting.'
      ;;

    test)
      dotnet build --configuration Release AIngle.sln

      # AINGLE-2442: Explictly set LANG to work around ICU bug in `dotnet test`
      LANG=en_US.UTF-8 dotnet test  --configuration Release --no-build \
          --filter "TestCategory!=Interop" AIngle.sln
      ;;

    perf)
      pushd ./src/apache/perf/
      dotnet run --configuration Release --framework net5.0
      ;;

    dist)
      # pack NuGet packages
      dotnet pack --configuration Release AIngle.sln

      # add the binary LICENSE and NOTICE to the tarball
      mkdir build/
      cp LICENSE NOTICE build/

      # add binaries to the tarball
      mkdir build/main/
      cp -R src/apache/main/bin/Release/* build/main/
      mkdir build/codegen/
      cp -R src/apache/codegen/bin/Release/* build/codegen/

      # build the tarball
      mkdir -p ${ROOT}/dist/csharp
      (cd build; tar czf ${ROOT}/../dist/csharp/aingle-csharp-${VERSION}.tar.gz main codegen LICENSE NOTICE)

      # build documentation
      doxygen AIngle.dox
      mkdir -p ${ROOT}/build/aingle-doc-${VERSION}/api/csharp
      cp -pr build/doc/* ${ROOT}/build/aingle-doc-${VERSION}/api/csharp
      ;;

    interop-data-generate)
      dotnet run --project src/apache/test/AIngle.test.csproj --framework net5.0 ../../share/test/schemas/interop.ain ../../build/interop/data
      ;;

    interop-data-test)
      LANG=en_US.UTF-8 dotnet test --filter "TestCategory=Interop"
      ;;

    clean)
      rm -rf src/apache/{main,test,codegen,ipc,msbuild,perf}/{obj,bin}
      rm -rf build
      rm -f  TestResult.xml
      ;;

    *)
      echo "Usage: $0 {lint|test|clean|dist|perf|interop-data-generate|interop-data-test}"
      exit 1

  esac

done
