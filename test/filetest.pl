use strict;
use warnings;

my $__MEM_TESTS_FAILED__ = 0;

sub assert_output {
    my ($string, $exp_out, $exp_stat) = @_;
    my (undef, $filename, $line) = caller;


    my $RED = "\x1B[31m";
    my $END = "\x1B[0m";

    my $output = qx+../YASL $string+;
    my $status = $?;
    my $exitcode = !($output eq $exp_out && $status == $exp_stat) || 0;

    if ($status != $exp_stat) {
            print $RED . "exitcode assert failed in $filename (line $line): $status =/= $exp_stat" . $END . "\n";
    }

    if ($output ne $exp_out) {
        print $RED . "output assert failed in $filename (line $line): $exp_out =/= $output" . $END . "\n";
    }

    $__MEM_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

while (defined(my $file = glob 'inputs/*.yasl')) {
    # print "Testing $file for leaks...\n";
    open my ($filename), $file;
    my $line = <$filename>;
    close $filename;
    $line =~ s/^##//;
    $line =~ s/\n$//;
    assert_output($file, eval '"' . $line . '"', 0);
}


exit $__MEM_TESTS_FAILED__;
