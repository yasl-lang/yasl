use strict;
use warnings;

my $__MEM_TESTS_FAILED__ = 0;

sub assert_output {
    my ($string, $exp_stat) = @_;
    my (undef, $filename, $line) = caller;


    my $RED = "\x1B[31m";
    my $END = "\x1B[0m";

    my $output = qx+valgrind --error-exitcode=1 --leak-check=full ../YASL inputs/$string+;
    my $status = $?;
    my $exitcode = ($status != $exp_stat) || 0;

    if ($status != $exp_stat) {
        print $RED . "exitcode assert failed in $filename (line $line): $status =/= $exp_stat" . $END . "\n";
    }

    $__MEM_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

assert_output('simple.yasl', 0);

exit $__MEM_TESTS_FAILED__;
