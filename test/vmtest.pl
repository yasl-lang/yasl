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


# Literals
assert_output(q+echo '\ttab'
               +,
              "\ttab\n", 0);
assert_output(q+echo `no escapes\a\b\f\n\r\t\v\0\'\\\\`
               +,
              'no escapes\a\b\f\n\r\t\v\0\\\'\\\\
', 0);
assert_output(q+let $x = 10
                let $y = 12
                echo "$x is #{$x}, #{$y}.";+,
              "\$x is 10, 12.\n",
              0);
assert_output(q+let $x = 10
                let $y = 12
                echo "$x is #{$x        }, #{$y}";+,
              "\$x is 10, 12\n",
              0);
assert_output(q+let $x = 10
                let $y = 12
                echo "$x is #{$x  }#{$y}  ";+,
              "\$x is 1012  \n",
              0);

# Comprehensions
assert_output(qq"let x = { x*2:-x for x <- [1, 2, 3, 4] if x % 2 == 0}
                 for i <- x {
                      echo i
                      echo x[i]
                 }\n",
              "4\n-2\n8\n-4\n", 0);
assert_output(qq"let x = { x*2:-x for x <- [1, 2, 3]}
                 for i <- x {
                      echo i
                      echo x[i]
                 }\n",
              "2\n-1\n4\n-2\n6\n-3\n", 0);

# Binary Operators
assert_output("echo 2 ** 4\n", "16\n", 0);
assert_output("echo 2 ** -1\n", "0.5\n", 0);
assert_output("echo -2 ** -1\n", "-0.5\n", 0);

assert_output("echo 5 * 6\n", "30\n", 0);
assert_output("echo 5 / 2\n", "2.5\n", 0);
assert_output("echo 6 // 2\n", "3\n", 0);
assert_output("echo 5 % 2\n", "1\n", 0);

assert_output("echo 10 + 12\n", "22\n", 0);
assert_output("echo 3.5 - 2\n", "1.5\n", 0);

assert_output("echo 1 || 2\n", "1\n", 0);
assert_output("echo 1 && 2\n", "2\n", 0);

assert_output("echo 'str1' < 'str2'\n", "true\n", 0);
assert_output("echo 'str1' > 'str2'\n", "false\n", 0);
assert_output("echo 'str1' <= 'str2'\n", "true\n", 0);
assert_output("echo 'str1' >= 'str2'\n", "false\n", 0);

assert_output("echo 'str1' < 'str12'\n", "true\n", 0);
assert_output("echo 'str1' <= 'str12'\n", "true\n", 0);
assert_output("echo 'str1' >= 'str12'\n", "false\n", 0);
assert_output("echo 'str1' > 'str12'\n", "false\n", 0);

assert_output("echo 'str1' > 'str1'\n", "false\n", 0);
assert_output("echo 'str1' < 'str1'\n", "false\n", 0);
assert_output("echo 'str1' >= 'str1'\n", "true\n", 0);
assert_output("echo 'str1' <= 'str1'\n", "true\n", 0);

assert_output("echo 'str1' == 'str2'\n", "false\n", 0);
assert_output("echo 'str1' == 'str12'\n", "false\n", 0);
assert_output("echo 'str1' == 'str1'\n", "true\n", 0);

# Ternary Operator
assert_output("echo true ? 1 : 0\n", "1\n", 0);
assert_output("echo false ? 1 : 0\n", "0\n", 0);

# If Statements
assert_output(qq"let x = true
                 if x { echo 1; } else { echo 0; };",
              "1\n", 0);
assert_output(qq"let x = false
                 if x { echo 1; } else { echo 0; };",
              "0\n", 0);
assert_output(qq"let x = 1
                 if x > 0 { echo '+'; } elseif x < 0 { echo '-'; } else { echo '0'; };",
              "+\n", 0);
assert_output(qq"let x = -1
                 if x > 0 { echo '+'; } elseif x < 0 { echo '-'; } else { echo '0'; };",
              "-\n", 0);
assert_output(qq"let x = 0
                 if x > 0 { echo '+'; } elseif x < 0 { echo '-'; } else { echo '0'; };",
              "0\n", 0);

# Functions
assert_output(qq"fn add(a, b) { return a + b; }
                 echo add(10, 11);",
              "21\n", 0);
assert_output(qq"let x = 10
                 fn add(a, b) { let sum = a + b; return sum; }
                 echo add(10, 11)
                 echo x;",
              "21\n10\n", 0);

# String Methods
assert_output("echo 'YASL'->__get(3)\n", "L\n", 0);
assert_output("echo 'YASL'[3]\n", "L\n", 0);
assert_output("echo 'YASL'->__get(-1)\n", "L\n", 0);

# List Methods
assert_output(qq"let x = [1, 2, 3]
                 x[1] = 0
                 echo x\n", "[1, 0, 3]\n", 0);
assert_output(qq"let x = [1, 2, 3]
                 echo x[0]
                 echo x[1]
                 echo x[2]\n", "1\n2\n3\n", 0);
assert_output(qq"let x = [0, 1, 2, 3, 4, 5, 6]
                 let y = x->slice(1, 5)
                 echo y\n", "[1, 2, 3, 4]\n", 0);
assert_output(qq"let x = []
                 x->push(x)
                 echo x
                 x->clear()\n", "[[...]]\n", 0);
 
# Table Methods
assert_output(qq"let y = []
                 let x = {}
                 x.x = x
                 y->push(y)
                 y->push(x)
                 x.y = y
                 echo x
                 echo y
                 x->clear()
                 y->clear()\n", "{x: {...}, y: [[...], {...}]}\n[[...], {x: {...}, y: [...]}]\n", 0);

# General
assert_output(qq"let x = []
                 let i = 0
                 while i < 100000 {
                     x->push(i)
                     i += 1
                 };",
              "",
              0);

assert_output(qq"let x = 10
                 let y = x + 10
                 echo x
                 echo y
                ",
              "10\n20\n",
              0);

assert_output(qq"const x = 10
                 const y = x + 10
                 echo x
                 echo y
                ",
              "10\n20\n",
              0);

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
