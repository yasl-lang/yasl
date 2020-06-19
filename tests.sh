#!/bin/bash

shopt -s globstar nullglob dotglob

declare -i failed=0;

[[ "$1" == "-m" ]];
declare MEM=$?;

run_mem_tests () {
    declare folder="$1";
    echo "Running memory tests on $folder...";
    for f in test/$folder/**/*.yasl; do
        valgrind --error-exitcode=1 --leak-check=full ./yasl $f > /dev/null 2>&1;
        declare exit_code=$?;
        if (( exit_code != 0 )); then
            echo "Memory Error in $f";
            (( ++failed ));
        fi;
    done;
}

run_tests () {
    declare folder="$1";
    declare expected_exit="$2";
    echo "Running tests on $folder...";
    declare ext;
    case "$expected_exit" in
        0) ext=".out" ;;
        *) ext=".err" ;;
    esac
    for f in test/$folder/**/*.yasl; do
        declare expected actual;
        expected=$(<"$f$ext");
        actual=$(./yasl "$f" 2>&1);
        declare exit_code=$?;
        if [[ "$expected" != "$actual" || $exit_code -ne $expected_exit ]]; then
            echo "Failed test for $f. Exited with $exit_code, expected $expected_exit.";
            echo "Expected:";
            echo "$expected"
            echo "Got:"
            echo "$actual"
            (( ++failed ));
        fi;
    done;
    if (( MEM == 0 )); then
        run_mem_tests $1;
    fi;
}

run_tests inputs 0
run_tests errors/assert 10
run_tests errors/stackoverflow 11
run_tests errors/type 5

echo "Failed $failed script tests.";

exit $failed;
