Benchmarks for YASL vs other scripting languages

Obviously, statically typed compiled languages are going to be much faster than YASL, so these benchmarks will mainly
focus on dynamically typed interpreted languages.

The benchmarks were run on my Linux machine, and took the mean over 5 runs each.
All times given are in seconds unless otherwise stated.

The following are the specific version of each language used:

Python:
Python 3.7.0 (default, Jun 28 2018, 13:15:42)
[GCC 7.2.0] :: Anaconda, Inc. on linux

YASL:
v0.10.3, built from source

Benchmark Results:

fib_rec:
YASL: 3.6492
Python: 3.2944

list_push:
YASL: 2.125
Python: 1.453