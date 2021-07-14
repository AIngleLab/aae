#!/bin/bash



set -e

case "$TRAVIS_OS_NAME" in
"linux")
    sudo apt-get -q update
    sudo apt-get -q install --no-install-recommends -y curl git gnupg-agent locales pinentry-curses pkg-config rsync software-properties-common
    sudo apt-get -q clean
    sudo rm -rf /var/lib/apt/lists/*

    # Only Yetus 0.9.0+ supports `ADD` and `COPY` commands in Dockerfile
    curl -L https://www-us.apache.org/dist/yetus/0.10.0/apache-yetus-0.10.0-bin.tar.gz | tar xvz -C /tmp/
    # A dirty workaround to disable the Yetus robot for TravisCI,
    # since it'll cancel the changes that .travis/script.sh will do,
    # even if the `--dirty-workspace` option is specified.
    rm /tmp/apache-yetus-0.10.0/lib/precommit/robots.d/travisci.sh
    ;;
"windows")
    # Install all (latest) SDKs which are used by multi framework projects
    choco install dotnetcore-2.1-sdk # .NET Core 2.1
    choco install dotnetcore-sdk     # .NET Core 3.1
    choco install dotnet-sdk         # .NET 5.0
    ;;
*)
    echo "Invalid PLATFORM"
    exit 1
    ;;
esac
