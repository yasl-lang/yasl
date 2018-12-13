# YASL
Bytecode Interpreter for Yet Another Scripting Language (YASL). More information can be found on my website, here: https://yasl-lang.github.io/docs/.

## Installation
YASL can be compiled from source with the following commands:
```bash
git clone --recurse-submodules https://github.com/yasl-lang/yasl.git
cd yasl
cmake --build .
```

You should then be able to type `./YASL -V` from within the `yasl` directory, which should print out the current version of YASL.

## Running YASL Code
A YASL script can be run using `./YASL filename`. This will search in the current directory (same directory as `YASL`) for a file named `filename`, and run it.

## Testing

```bash
$> chmod +x tests
$> ./tests
```

Tests should execute with exitcode 0. If they do not, either there's a bug or your installation was incorrect.
