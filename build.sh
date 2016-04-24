#!/bin/sh
if [ "$#" -ne 1 ]
then
  echo "Usage: sh build path_to_deadbeef.h"
  exit 1
fi
if [ ! -d "$1" ]; then
  echo "$1 is not a directory."
  echo "Usage: sh build path_to_dir_with_deadbeef.h"
  exit 1
fi
gcc -Wall -I$1 -fPIC -std=c99 -shared -O2 -o rating.so rating.c && echo "Built rating.so"
