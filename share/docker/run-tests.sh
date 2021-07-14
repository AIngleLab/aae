#!/bin/bash



headline(){
  echo -e "\e[1;34m#################################################################"
  echo -e "##### $1 \e[1;37m"
  echo -e "\e[1;34m#################################################################\e[0m"
}

set -e

for lang in /aingle/lang/*/
do
  headline "Run tests: $lang"
  cd "$lang"
  ./build.sh lint test
done
