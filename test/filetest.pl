use strict;
use warnings;

my $__MEM_TESTS_FAILED__ = 0;

sub assert_output {
    my ($string, $exp_out, $exp_stat) = @_;

    my $RED = "\x1B[31m";
    my $END = "\x1B[0m";

    my $output = qx+../YASL $string+;
    my $status = $?;
    my $exitcode = !($output eq $exp_out && $status == $exp_stat) || 0;

    if ($status != $exp_stat) {
            print $RED . "exitcode assert failed in $string: $status =/= $exp_stat" . $END . "\n";
    }

    if ($output ne $exp_out) {
        print $RED . "output assert failed in $string: $exp_out =/= $output" . $END . "\n";
    }

    $__MEM_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

while (defined(my $file = glob 'inputs/*.yasl')) {
    # print "Testing $file for leaks...\n";
    local $/;
    open my ($filename), "$file.out";
    my $line = <$filename>;
    close $filename;
    assert_output($file, $line, 0);
}


exit $__MEM_TESTS_FAILED__;
