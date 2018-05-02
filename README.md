# YASL
Bytecode Interpreter for Yet Another Scripting Language (YASL). More information can be found on my website, here: https://coffeetableespresso.github.io/yasl.

## Building Interpreter
The interpreter can be compiled using the Makefile via the command `make YASL`.
The environment can be cleaned of all binaries (object files and YASL) with `make clean`.

## Compiling to YASL Bytecode
A YASL file (`*.yasl` or `*.ysl`) can be compiled to a YASL bytecode file (`*.yb`) by running `python3 -m compiler.main example.ysl`. The default output is a YASL bytecode file named `source.yb`.

## Running YASL Bytecode
Once compiled to bytecode, a YASL script can be run using `./YASL example.yb` or `./YASL`. The second is equivalent to running `./YASL source.yb`, with`source.yb` being the default name that a YASL script is compiled to.
