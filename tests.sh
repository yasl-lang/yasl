#!/bin/bash

if [ -n "$ZSH_NAME" ]; then
    setopt extended_glob;
else
    shopt -s globstar nullglob dotglob;
fi;

declare K_RED="\033[31m";
declare K_END="\033[0m";
declare -i failed=0;
declare -i ran=0;
read -r -d '' usage << 'EOF'
usage: yasl [option] [input]
options:
	-C: checks `input` for syntax errors but doesn't run it.
	-e input: executes `input` as code and prints result of last statement.
	-E input: executes `input` as code.
	-h: show this text.
	-V: print current version.
	input: name of file containing script (or literal to execute with -e or -E).
EOF

[[ "$1" == "-m" ]];
declare NO_MEM=$?;

run_mem_tests () {
    declare folder="$1";
    echo "Running memory tests in $folder...";
    for f in test/$folder/**/*.yasl; do
        valgrind --error-exitcode=-1 --leak-check=full --exit-on-first-error=yes ./yasl $f > /dev/null 2>&1;
        declare exit_code=$?;
        if (( exit_code == 255 )); then
            case ${f%.yasl} in
            test/inputs/scripts/fib_match|test/inputs/scripts/fib|test/errors/stackoverflow/fib|test/errors/stackoverflow/list_eq)
                (( ++skipped ));
                ;;
            *)
                >&2 echo "Memory Error in $f";
                (( ++failed ));
                ;;
            esac;
        fi;
    done;
}

run_tests () {
    declare folder="$1";
    declare expected_exit="$2";
    echo "Running tests in $folder...";
    if [[ ! -d "test/$folder" ]]; then
        >&2 echo "Error: could not find $folder";
        exit -1;
    fi;
    declare ext;
    case "$expected_exit" in
        0) ext=".out" ;;
        *) ext=".err" ;;
    esac;
    for f in test/$folder/**/*.yasl; do
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
    if (( NO_MEM != 0 )); then
        run_mem_tests $1;
    fi;
}

run_cli_test () {
    declare expected="$1";
    shift;
    declare torun="./yasl $@";
    declare actual=$(./yasl "$@");
    if [[ "$expected" != "$actual" ]]; then
        >&2 echo -e "Failed test $K_RED$torun$K_END.";
        >&2 echo "Expected:";
        >&2 echo "$expected";
        >&2 echo "Got:";
        >&2 echo "$actual";
        (( ++failed ));
    fi;
    (( ++ran ));
}

run_cli_tests () {
    echo "Running CLI tests...";
    run_cli_test 'YASL v0.11.9' '-V';
    run_cli_test '10' '-e' "let x=10; x;";
    run_cli_test '' '-E' 'let x = 10; x;';
    run_cli_test '10' '-E' 'let x = 10; echo x;';
    run_cli_test "$usage" '-h';
}

run_tests inputs 0;
run_tests errors/assert 10;
run_tests errors/error 2;
run_tests errors/stackoverflow 11;
run_tests errors/type 5;
run_tests errors/divisionbyzero 6;
run_tests errors/syntax 4;
run_tests errors/value 7;
run_cli_tests;

echo "Passed $(( ran - failed ))/$(( ran )) script tests. (Skipped $((skipped)).)";

exit $failed;
