# YASL
Bytecode Interpreter for Yet Another Scripting Language (YASL). More information can be found on my website, here: https://github.com/CoffeeTableEspresso/YASL/wiki.

## Building Interpreter
The bytecode interpreter can be compiled using the included `CMakeLists`.
The Makefile included is slightly out of date, but should work with minor changes.

## Running YASL Code
A YASL script can be run using `./YASL`. This will search in the current directory (same directory as `YASL`) for a file named `sample.ysl`, and compile then vm_run it.
