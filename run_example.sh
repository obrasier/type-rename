#!/bin/bash
EXE=type-rename
FILE="$1"
shift
remainder="$*"
if [[ -z "${FILE}" ]]; then
  echo "Please enter a file to change, like the following:"
  echo "$0 my_file.cpp"
  exit 1
fi
echo "Making backup in $FILE.bak"
cp $FILE $FILE.bak
./$EXE $FILE -- $remainder

