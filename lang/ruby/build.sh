#!/bin/bash



set -e

# connect to aingle ruby root directory
cd "$(dirname "$0")"

# maintain our gems here
export GEM_HOME="$PWD/.gem/"
export PATH="/usr/local/rbenv/shims:$GEM_HOME/bin:$PATH"

# bootstrap bundler
gem install --no-document -v 1.17.3 bundler

# rbenv is used by the Dockerfile but not the Github action in CI
rbenv rehash 2>/dev/null || echo "Not using rbenv"
bundle install

for target in "$@"
do
  case "$target" in
    lint)
      bundle exec rubocop
      ;;

    test)
      bundle exec rake test
      ;;

    dist)
      bundle exec rake dist
      ;;

    clean)
      bundle exec rake clean
      rm -rf tmp aingle.gemspec data.avr
      ;;

    *)
      echo "Usage: $0 {lint|test|dist|clean}"
      exit 1
  esac
done
