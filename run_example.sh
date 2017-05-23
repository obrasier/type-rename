#!/bin/bash
EXE=type-rename
FILE="$1"
shift
remainder="$*"
if [[ ! -f ${FILE} ]]; then
  echo "Please enter a file to change, like the following:"
  echo "$0 my_file.cpp"
  echo "or add includes..."
  echo "$0 my_file.cpp -std=c++11 -I/usr/lib/llvm-3.8/bin/../lib/clang/3.8.1/include"
  exit 1
fi

./$EXE $FILE -- $remainder

