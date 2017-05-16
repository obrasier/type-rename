Type Rename
===========


This is a clang tool to rename the varible types in a give C file. It ignores all #include files

This works on llvm-clang 3.8. It currently works on:

- function declarations
- function parameters
- variable declarations

Currently renames all variables to be the width expected in Arduino. Could be used to change any variable type to any other by editing the std::pair at the top of `TypeRename.cpp`

To compile the tool just run:

```bash
make
```

Then run:

```bash
./run_example.sh my_file.cpp
```

It will scan the AST of `my_file.cpp` and change the variables to what is specified in the pair at the top of `TypeRename.cpp`

If you need to pass arguments to the `my_file.cpp` for execution (for example `-std=c++11` or include directories), just add them afterwards, for example:

```bash
./run_example.sh my_file.cpp -std=c++11 -I../path/to/include/dir -I/usr/lib/llvm-3.8/bin/../lib/clang/3.8.1/include
```
