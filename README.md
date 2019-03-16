# YASL
Bytecode Interpreter for Yet Another Scripting Language (YASL). More information can be found here: https://yasl-lang.github.io/docs/.

## Installation
YASL can be compiled from source with the following commands:
```bash
git clone https://github.com/yasl-lang/yasl.git
cd yasl
cmake --configure .
cmake --build .
```

You should then be able to type `./YASL -V` from within the `yasl` directory, which should print out the current version of YASL. YASL is not installed on your path, although doing so is easy.

## Running YASL Code
A YASL script can be run using `./YASL filename`. This will search in the current directory (same directory as `YASL`) for a file named `filename`, and run it.

An online Interpreter is also available here: https://coffeetableespresso.github.io/YASL.html. It's the same interpreter as found in this repo, but compiled to WASM. It might be slow and buggy compared to the native interpreter, but we're working to fix that.

## Running Tests

```bash
$> chmod +x tests
$> ./tests
```

Tests should execute with exitcode 0. If they do not, either there's a bug or your installation was incorrect.

## Feature Requests
If you find yourself desiring a feature that YASL is missing, please open an issue on this repo with the `feature-request` tag, and outline what feature you want. There's no need to provide an implementation at this stage, just explain on the issue what you want added. This doesn't guarantee that I will add it. If I do, I'll open another isssue asking for an implementation of the feature, which can then be added to YASL once it's complete.
