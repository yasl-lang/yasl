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
        # print "$file\n";
        local $/;
        open my ($filename), "$file.out";
        my $line = <$filename>;
        close $filename;
        assert_output($file, $line, 0);
    }
    foreach my $file (@subdirs) {
        process_dir($file);
    }
}

process_dir('inputs/*');

exit $__MEM_TESTS_FAILED__;
