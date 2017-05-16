#!/bin/sh
EXE=type-rename
FILE=sketch_may09a.ino.cpp
cp $FILE $FILE.bak
./$EXE $FILE -- -std=c++11 -I../../esplora-sim/src/inc -I/usr/lib/llvm-3.8/bin/../lib/clang/3.8.1/include

