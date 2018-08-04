use strict;
use warnings;

my $__YASL_TESTS_FAILED__ = 0;

sub assert_output {
    my ($package, $filename, $line) = caller;
    my ($string, $exp_out, $exp_stat) = @_;
    my $RED = "\x1B[31m";
    my $END = "\x1B[0m";

    open(my $fh, '>', '../cmake-build-debug/dump.ysl') or die "Could not open file";
    print $fh "$string";
    close $fh;

    my $output = `../cmake-build-debug/YASL ../cmake-build-debug/dump.ysl`;
    my $status = $?;
    my $exitcode = !($output eq $exp_out && $status == $exp_stat) || 0;

    if ($output ne $exp_out) {
        print $RED . "output assert failed in $filename (line $line): $exp_out =/= $output" . $END . "\n";
    }
    if ($status != $exp_stat) {
        print $RED . "exitcode assert failed in $filename (line $line): $status =/= $exp_stat" . $END . "\n";
    }

    $__YASL_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

# print assert_output("print 'thiabaud'::toupper()\n", "THABAUD\n", 0);

1;
