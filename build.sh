#!/bin/sh
USAGE="Usage: sh build /path/to/dir/with/deadbeef.h"
if [ "$#" -ne 1 ]
then
  echo "$USAGE"
  exit 1
fi
if [ ! -d "$1" ]; then
  echo "$1 is not a directory."
  echo "$USAGE"
  exit 1
fi
gcc -Wall -I$1 -fPIC -std=c99 -shared -O2 -o rating.so rating.c && echo "Built rating.so"
