# YASL
Bytecode Interpreter for Yet Another Scripting Language (YASL).
An online interpreter (that you can use to try YASL from your browser without downloading anything) can be found [here](https://yasl-lang.github.io/playground/interpreter.html), or you can view the docs [here](https://yasl-lang.github.io/docs/).

## Portability
YASL is written in C99, and compiled and tested on Windows (MinGW and MSVC), Ubuntu (GCC, G++, Clang, Clang++, TCC), and MacOS (Clang, Clang++).
Tests are run on all platforms using Azure for CI. Additional tests for the CLI are also run on all non-Windows systems.
If you find a C99 compiler that YASL is not compatible with, please open an issue so I can do my best to fix this.
YASL should eventually be completely compatible with any C99 compiler on any platform.

## Platform Specific Code
The only platform-specific code found in YASL is for dynamically loading YASL modules written in C (as opposed to including them when compiling YASL).
This relies on `dlopen` for POSIX systems, and the Windows API for this on Windows. If you find a platform for which either of these doesn't work, please open an issue and I'll do my best to fix this.

## Dependencies
The only dependencies YASL has are `dlopen.h` on POSIX systems and `windows.h` on Windows.
If either of these is a problem on your platform, these can also be disabled (although you won't be able to dynamically load YASL modules).

Besides this, YASL uses no features not found in standard C99.

## Installation
YASL can be compiled from source with the following commands:
```bash
git clone https://github.com/yasl-lang/yasl.git
cd yasl
cmake .
cmake --build .
```

You should then be able to type `./yasl -V` from within the `yasl` directory, which should print out the current version of YASL. YASL is not installed on your path, although doing so is easy. 

`install.sh`, which is included in this repo, will install `yasl` to `/usr/local/bin`, as well as install the headers/static library needed to embed YASL in a C or C++ project. This should work on all POSIX systems.

## Running YASL Code
A YASL script can be run using `yasl filename`. A REPL can be entered by typing `yasl` without any arguments. Other options can be seen by typing `yasl -h`.

## Running Tests

The following can be used to execute the tests:

```bash
$> ./tests.sh
$> ./tests
```

Note that `tests.sh` requires either Bash 4 or Z Shell in order to run. By default, it runs with `bash`, but you can
run `zsh tests.sh` if Bash 4 isn't available on your system (MacOS for example).

`tests.sh` tests the YASL interpreter itself, as it would be invoked from the terminal. `tests` tests the YASL API itself + unittests.

All tests should execute with exitcode 0. If they do not, either there's a bug or your installation was incorrect. Either way, please open an issue if you can't figure it out.

## Feature Requests
If you find yourself desiring a feature that YASL is missing, please open an issue on this repo with the `feature-request` tag, and outline what feature you want. There's no need to provide an implementation at this stage, just explain on the issue what you want added. This doesn't guarantee that I will add it. If I do, I'll open another isssue asking for an implementation of the feature, which can then be added to YASL once it's complete.

Keep in mind that YASL is meant to be small, to facilitate easy embedding. As such, I'm fairly picky about new features, especially if they involve changes to the syntax. If a feature can be implemented as a library, I'm much more receptive, since libraries can be excluded from YASL fairly easily if they're not needed, whereas syntax cannot.
