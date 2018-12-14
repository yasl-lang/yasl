use strict;
use warnings;

my $__MEM_TESTS_FAILED__ = 0;

sub assert_output {
    my ($string, $exp_stat) = @_;
    my (undef, $filename, $line) = caller;


    my $RED = "\x1B[31m";
    my $END = "\x1B[0m";

    my $output = qx+valgrind --error-exitcode=1 --leak-check=full ../YASL $string+;
    my $status = $?;
    my $exitcode = ($status != $exp_stat) || 0;

    if ($status != $exp_stat) {
        print $RED . "memory leak in $string." . $END . "\n";
    }

    $__MEM_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

while (defined(my $file = glob 'inputs/*.yasl')) {
    # print "Testing $file for leaks...\n";
    assert_output($file, 0);
}

exit $__MEM_TESTS_FAILED__;
