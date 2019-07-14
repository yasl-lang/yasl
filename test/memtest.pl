use strict;
use warnings;

my $__MEM_TESTS_FAILED__ = 0;

sub assert_output {
    my ($string, $exp_stat) = @_;

    my $RED = "\x1B[31m";
    my $END = "\x1B[0m";

    my $output = qx+valgrind --error-exitcode=1 --leak-check=full ../YASL $string 2>/dev/null+;
    my $status = $?;
    my $exitcode = ($status != $exp_stat) || 0;

    if ($status != $exp_stat) {
        print $RED . "memory leak in $string." . $END . "\n";
    }

    $__MEM_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

sub process_dir {
    my ($dir) = @_;
    my @subdirs = ();
    while (defined(my $file = glob $dir)) {
        if (-d $file) {
            push(@subdirs, "$file/*");
            next;
        }
        if ($file !~ /.*\.yasl$/) {
            next;
        }
        # print "Testing $file for leaks...\n";
        assert_output($file, 0);
    }
    foreach my $file (@subdirs) {
        process_dir($file);
    }
}

process_dir('inputs/*');

exit $__MEM_TESTS_FAILED__;
