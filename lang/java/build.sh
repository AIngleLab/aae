#!/bin/bash



set -e

usage() {
  echo "Usage: $0 {lint|test|dist|clean}"
  exit 1
}

main() {
  local target
  (( $# )) || usage
  for target; do
    case "$target" in
      lint)
        mvn -B spotless:apply
        ;;
      test)
        mvn -B test
        # Test the modules that depend on hadoop using Hadoop 2
        mvn -B test -Phadoop2
        ;;
      dist)
        mvn -P dist package -DskipTests javadoc:aggregate
        ;;
      clean)
        mvn clean
        ;;
      *)
        usage
        ;;
    esac
  done
}

main "$@"
