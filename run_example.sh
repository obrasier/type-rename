#!/bin/bash
EXE=type-rename
FILE=$1
if [[ -z "${FILE}" ]]; then
  echo "Please enter a file to change, like the following:"
  echo "$0 my_file.cpp"
  exit 1
fi
echo "Making backup in $FILE.bak"
cp $FILE $FILE.bak
./$EXE $FILE -- -std=c++11 -I../../esplora-sim/src/inc -I/usr/lib/llvm-3.8/bin/../lib/clang/3.8.1/include

