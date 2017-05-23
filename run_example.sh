#!/bin/bash
EXE=type-rename
FILE="$1"
shift
remainder="$*"
if [[ ! -f ${FILE} ]]; then
  echo "Please enter a file to change, like the following:"
  echo "$0 my_file.cpp"
  exit 1
fi

./$EXE $FILE -- $remainder

