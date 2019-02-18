use strict;
use warnings;

my $__CLI_TESTS_FAILED__ = 0;

sub assert_output {
    my ($string, $exp_out, $exp_stat) = @_;
    my (undef, $filename, $line) = caller;

    my $RED = "\x1B[31m";
    my $END = "\x1B[0m";

    my $output = qx+../$string+;
    my $status = $?;
    my $exitcode = !($output eq $exp_out && $status == $exp_stat) || 0;

    if ($output ne $exp_out) {
        print $RED . "output assert failed in $filename (line $line): $exp_out =/= $output" . $END . "\n";
    }
    if ($status != $exp_stat) {
        print $RED . "exitcode assert failed in $filename (line $line): $status =/= $exp_stat" . $END . "\n";
    }

    $__CLI_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

assert_output("YASL -V", "YASL v0.3.6\n", 0);
assert_output("YASL -h",
              "usage: yasl [option] [file]\n" .
              "options:\n" .
              "\t-h: this menu\n" .
              "\t-V: print current version\n" .
              "\tfile: name of file containing script\n",
              0);

exit $__CLI_TESTS_FAILED__;
