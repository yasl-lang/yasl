#!/bin/bash

shopt -s globstar nullglob dotglob;

declare K_RED="\033[31m";
declare K_END="\033[0m";
declare -i failed=0;
declare -i ran=0;

[[ "$1" == "-m" ]];
declare MEM=$?;

run_mem_tests () {
    declare folder="$1";
    echo "Running memory tests in $folder...";
    for f in test/$folder/**/*.yasl; do
        valgrind --error-exitcode=-1 --leak-check=full ./yasl $f > /dev/null 2>&1;
        declare exit_code=$?;
        if (( exit_code == -1 )); then
            >&2 echo "Memory Error in $f";
            (( ++failed ));
        fi;
    done;
}

run_tests () {
    declare folder="$1";
    declare expected_exit="$2";
    echo "Running tests in $folder...";
    declare ext;
    case "$expected_exit" in
        0) ext=".out" ;;
        *) ext=".err" ;;
    esac;
    for f in test/$folder/**/*.yasl; do
        declare expected actual;
        expected=$(<"$f$ext");
        actual=$(./yasl "$f" 2>&1);
        declare exit_code=$?;
        if [[ "$expected" != "$actual" || $exit_code -ne $expected_exit ]]; then
            >&2 echo -e "Failed test for $K_RED$f$K_END. Exited with $exit_code, expected $expected_exit.";
            >&2 echo "Expected:";
            >&2 echo "$expected";
            >&2 echo "Got:";
            >&2 echo "$actual";
            (( ++failed ));
        fi;
        (( ++ran ));
    done;
    if (( MEM == 0 )); then
        run_mem_tests $1;
    fi;
}

run_tests inputs 0;
run_tests errors/assert 10;
run_tests errors/stackoverflow 11;
run_tests errors/type 5;

echo "Passed $(( ran - failed ))/$ran script tests.";

exit $failed;
