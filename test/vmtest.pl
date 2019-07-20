use strict;
use warnings;

my $__VM_TESTS_FAILED__ = 0;
my $RED = "\x1B[31m";
my $END = "\x1B[0m";

sub assert_output {
    my ($string, $exp_out, $exp_stat) = @_;
    my (undef, $filename, $line) = caller;

    my $debug_dump = '/dump.ysl';
    my $debug_yasl = '/YASL';

    open(my $fh, '>', '..' . $debug_dump) or die "Could not open file $debug_dump";
    print $fh "$string";
    close $fh;

    my $output = qx/"..$debug_yasl" "..$debug_dump"/;
    my $status = $? >> 8;
    my $exitcode = !($output eq $exp_out && $status == $exp_stat) || 0;

    if ($output ne $exp_out) {
        print $RED . "output assert failed in $filename (line $line): $exp_out =/= $output" . $END . "\n";
    }
    if ($status != $exp_stat) {
        print $RED . "exitcode assert failed in $filename (line $line): $status =/= $exp_stat" . $END . "\n";
    }

    $__VM_TESTS_FAILED__ ||= $exitcode;
    return $exitcode;
}

# Errors

my $YASL_SYNTAX_ERROR = 4;
my $YASL_TYPE_ERROR = 5;
my $YASL_DIVISION_BY_ZERO_ERROR = 6;

assert_output(qq"for let x = 0; x < 5; x += 1 { };
                 echo x;", $RED . "SyntaxError: Undeclared variable x (line 2).\n" . $END, $YASL_SYNTAX_ERROR);
assert_output(qq"const x = 10; x = 11;", $RED . "SyntaxError: Cannot assign to constant x (line 1).\n" . $END, $YASL_SYNTAX_ERROR);
assert_output(qq"const x = 10; let x = 11;", $RED . "SyntaxError: Illegal redeclaration of x (line 1).\n" . $END, $YASL_SYNTAX_ERROR);
assert_output(qq"let x = 10; let x = 11;", $RED . "SyntaxError: Illegal redeclaration of x (line 1).\n" . $END, $YASL_SYNTAX_ERROR);
assert_output(q"let x = [ b for b <- [1, 2, 3, 4] if b % 2 == 0 ]; echo b;",
              $RED . "SyntaxError: Undeclared variable b (line 1).\n" . $END, $YASL_SYNTAX_ERROR);
assert_output("echo if;",
              $RED . "SyntaxError: ParsingError in line 1: expected expression, got `if`\n" . $END, $YASL_SYNTAX_ERROR);
assert_output("x;", $RED . "SyntaxError: Undeclared variable x (line 1).\n" . $END, $YASL_SYNTAX_ERROR);
assert_output("echo 'hello \\o world'\n", $RED . "SyntaxError: Invalid string escape sequence in line 1.\n" . $END, $YASL_SYNTAX_ERROR);
assert_output("echo 'hello \\xworld'\n", $RED . "SyntaxError: Invalid hex string escape in line 1.\n" . $END, $YASL_SYNTAX_ERROR);

assert_output("echo true + false;",
              $RED . "TypeError: + not supported for operands of types bool and bool.\n" . $END, $YASL_TYPE_ERROR);

assert_output(qq"echo 1 // 0;", $RED . "DivisionByZeroError\n" . $END, $YASL_DIVISION_BY_ZERO_ERROR);
assert_output(qq"echo 1 % 0;", $RED . "DivisionByZeroError\n" . $END, $YASL_DIVISION_BY_ZERO_ERROR);

exit $__VM_TESTS_FAILED__;
