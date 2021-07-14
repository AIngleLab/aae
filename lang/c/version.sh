#!/bin/bash
#

#

# This script is used to generate version numbers for autotools
#
# The information for libtool is maintained manually since
# the public API for the C library can change independent of the project
#
# Do each of these steps in order and libtool will do the right thing
# (1) If there are changes to libaingle:
#         libaingle_micro_version++
#         libaingle_interface_age++
#         libaingle_binary_age++
# (2) If any functions have been added:
#         libaingle_interface_age = 0
# (3) If backwards compatibility has been broken:
#         libaingle_binary_age = 0
#         libaingle_interface_age = 0
#
libaingle_micro_version=23
libaingle_interface_age=0
libaingle_binary_age=0

# IGNORE EVERYTHING ELSE FROM HERE DOWN.........
if test $# != 1; then
  echo "USAGE: $0 CMD"
    echo "  where CMD is one of: project, libtool, libcurrent, librevision, libage"
  exit 1
fi

# https://www.sourceware.org/autobook/autobook/autobook_91.html
# 'Current' is the most recent interface number that this library implements
libcurrent=$(($libaingle_micro_version - $libaingle_interface_age))
# The implementation number of the 'current' interface
librevision=$libaingle_interface_age
# The difference between the newest and oldest interfaces that this library implements
# In other words, the library implements all the interface numbers in the range from
# number 'current - age' to current
libage=$(($libaingle_binary_age - $libaingle_interface_age))

if test "$1" = "project"; then
  project_ver="undef"
  version_file="VERSION.txt"
  if test -f $version_file; then
    project_ver=$(cat $version_file)
  else
    version_file="../../share/VERSION.txt"
    if test -f $version_file; then
      project_ver=$(cat $version_file)
    fi
  fi
  printf "%s" $project_ver
elif test "$1" = "libtool"; then
  # useful for the -version-info flag for libtool
  printf "%d:%d:%d" $libcurrent $librevision $libage
elif test "$1" = "libcurrent"; then
  printf "%d" $libcurrent
elif test "$1" = "librevision"; then
  printf "%d" $librevision
elif test "$1" = "libage"; then
  printf "%d" $libage
fi
