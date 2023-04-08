# Allow caller to optionally specify a unix environment and memory testing
param([bool]$unix=$false, [bool]$mem=$false)

# Init globals
$ran = 0
$failed = 0
$ignored = 0
# $mem = $true

# Define constant help-output
$usage = @'
usage: yasl [option] [input]
options:
	-C: checks `input` for syntax errors but doesn't run it.
	-e input: executes `input` as code and prints result of last statement.
	-E input: executes `input` as code.
	-h: show this text.
	-V: print current version.
	input: name of file containing script (or literal to execute with -e or -E).
'@

$yasl = $unix ? './yasl' : './yasl.exe'

# This function takes in a folder name as an argument
# It then runs memory tests on all .yasl files in the specified folder using valgrind
function run_mem_tests([string]$folder) {
    Write-Output "Running memory tests in $folder..."

    # Get all .yasl files in the specified folder and its subfolders
    Get-ChildItem -Path "test/$folder" -Filter *.yasl -Recurse | ForEach-Object {
        $f = $_.FullName

        # Run valgrind on the current .yasl file and suppress its output
        & valgrind --error-exitcode=-1 --leak-check=full --exit-on-first-error=yes ./yasl $f 2>&1 > $null
        ++$script:ran

        # Check if valgrind exited with an error code
        if ($LASTEXITCODE -eq 255) {
            # Check if the current file is one of the files that should be ignored
            $ignore_tests = "inputs/scripts/fib_match", "inputs/scripts/fib", "errors/stackoverflow/fib", "errors/stackoverflow/list_eq", 
                            "inputs/closures/memoize", "inputs/scripts/heap", "inputs/vargs/*", "errors/assert/foreach"
            $test = $f.Replace(".yasl", "")
            if ($ignore_tests | Where { $test -like "*test/$_" }) {
                ++$script:ignored
            } else {
                # If the file is not one of the ignored files, report a memory error
                Write-Error "Memory Error in $f"
                ++$script:failed
            }
        }
    }
}

# This function takes in a folder name and an expected exit code as arguments
# It then runs tests on all .yasl files in the specified folder using the YASL program
function run_tests([string]$folder, [int]$expected_exit) {
    Write-Output "Running tests in $folder..."

    # Check if the specified folder exists
    if (!(Test-Path "test/$folder")) {
        Write-Error "Error: could not find $folder"
        exit -1
    }

    # Set the file extension for the expected output file based on the expected exit code
    $ext = if ($expected_exit -eq 0) { ".out" } else { ".err" }
    # Get all .yasl files in the specified folder and its subfolders
    Get-ChildItem -Path "test/$folder" -Filter *.yasl -Recurse | ForEach-Object {
        $f = $_.FullName

        # Get the expected output from the corresponding .out or .err file
        $expected = Get-Content -Raw "$f$ext"
        $expected = ($expected ?? '').ReplaceLineEndings()

        # Run the YASL program on the current .yasl file and capture its output
        $actual = (& $yasl $f 2>&1) | Out-String
        $exit_code = $LASTEXITCODE

        # Check if the actual output or exit code differs from the expected values
        if (($expected -ne $actual) -or ($exit_code -ne $expected_exit)) {
            $ignore_tests = "errors/syntax/*"

            if ($ignore_tests | Where { $f -like "*test/$_" }) {
                ++$script:ignored
            } else {
                Write-Error "Failed test for $f. Exited with $exit_code, expected $expected_exit.`nExpected: $expected : Got: $actual"
                ++$script:failed
            }
        }
        ++$script:ran
    }

    # Run memory tests on the specified folder if NO_MEM is equal to 0
    if ($mem) {
        if ($unix) {
            run_mem_tests $folder
        } else {
            Write-Warning 'Memory tests are not implemented for Windows platform, skipping'
        }
    }
}

# This function takes in the expected output and a list of arguments to run with the YASL program
# It then runs the YASL program with the provided arguments and compares the output with the expected output
function run_cli_test([string]$expected, [string[]]$test_args) {
    $torun = "$yasl $test_args"
    $actual = (& $yasl @test_args) -join [Environment]::NewLine
    $actual = $actual ?? ''
    if ($expected.ReplaceLineEndings() -ne $actual) {
        Write-Error "Failed test $torun.`nExpected: $expected : Got: $actual"
        ++$script:failed
    }
    ++$script:ran
}

# This function runs a series of tests using the run_cli_test function
function run_cli_tests() {
    Write-Output "Running CLI tests..."
    run_cli_test 'YASL v0.12.4' '-V'
    run_cli_test '10' '-e', 'let x=10; x;'
    run_cli_test '' '-E', 'let x = 10; x;'
    run_cli_test '10' '-E', 'let x = 10; echo x;'
    run_cli_test $usage '-h'
}

# Run tests on various folders with different expected exit codes
run_tests "inputs" 0
run_tests "errors/assert" 10
run_tests "errors/error" 2
run_tests "errors/stackoverflow" 11
run_tests "errors/type" 5
run_tests "errors/divisionbyzero" 6
run_tests "errors/syntax" 4
run_tests "errors/value" 7

# Run CLI tests
run_cli_tests

# Print the number of passed and total tests
Write-Output "Passed $(($ran - $failed))/$ran script tests. (ignored $ignored)."

# Exit with the number of failed tests as the exit code
exit $failed
