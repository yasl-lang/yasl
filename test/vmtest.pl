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

sub binop_error_message {
    my ($op, $left, $right) = @_;
    return $RED . "TypeError: " . $op . " not supported for operands of types " .
        $left . " and " . $right . ".\n" . $END;
}

sub unop_error_message {
    my ($op, $type) = @_;
    return $RED . "TypeError: " . $op . " not supported for operand of type " . $type . ".\n" . $END;
}

sub method_error_message {
    my ($name, $arg, $expected, $actual) = @_;
    return $RED . "TypeError: " . $name . " expected arg in position " .
        $arg . " to be of type " . $expected . ", got arg of type " . $actual . ".\n" . $END;
}

my $YASL_SYNTAX_ERROR = 4;
my $YASL_TYPE_ERROR = 5;
my $YASL_DIVISION_BY_ZERO_ERROR = 6;
my $YASL_VALUE_ERROR = 7;

# SyntaxError
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

# TypeError (Operators)
assert_output("echo .true | false;", binop_error_message("|", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true ^ false;", binop_error_message("^", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true &^ false;", binop_error_message("&^", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true & false;", binop_error_message("&", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true >> false;", binop_error_message(">>", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true << false;", binop_error_message("<<", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true + false;", binop_error_message("+", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true - false;", binop_error_message("-", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true * false;", binop_error_message("*", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true / false;", binop_error_message("/", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true // false;", binop_error_message("//", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true % false;", binop_error_message("%", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo .true ** false;", binop_error_message("**", "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo -true;", unop_error_message("-", "bool"), $YASL_TYPE_ERROR);
assert_output("echo +true;", unop_error_message("+", "bool"), $YASL_TYPE_ERROR);
assert_output("echo len true;", unop_error_message("len", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ^true;", unop_error_message("^", "bool"), $YASL_TYPE_ERROR);

# TypeError (Bool Methods)
assert_output("echo true.tostr(0);", method_error_message("bool.tostr", 0, "bool", "int"), $YASL_TYPE_ERROR);
assert_output("echo true.tobool(0);", method_error_message("bool.tobool", 0, "bool", "int"), $YASL_TYPE_ERROR);

# TypeError (Float Methods)
assert_output("echo 0.0.tostr(1);", method_error_message("float.tostr", 0, "float", "int"), $YASL_TYPE_ERROR);
assert_output("echo 0.0.toint(1);", method_error_message("float.toint", 0, "float", "int"), $YASL_TYPE_ERROR);
assert_output("echo 0.0.tofloat(1);", method_error_message("float.tofloat", 0, "float", "int"), $YASL_TYPE_ERROR);

# TypeError (Int Methods)
assert_output("echo 0.tostr(1.0);", method_error_message("int.tostr", 0, "int", "float"), $YASL_TYPE_ERROR);
assert_output("echo 0.tofloat(1.0);", method_error_message("int.tofloat", 0, "int", "float"), $YASL_TYPE_ERROR);
assert_output("echo 0.toint(1.0);", method_error_message("int.toint", 0, "int", "float"), $YASL_TYPE_ERROR);

# Type Error (List Methods)
assert_output("echo [].push(1, true);", method_error_message("list.push", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].copy(true);", method_error_message("list.copy", 0, "list", "bool"), $YASL_TYPE_ERROR);
assert_output("echo [].__add(true, []);", method_error_message("list.__add", 0, "list", "bool"), $YASL_TYPE_ERROR);
assert_output("echo [].__add(true, 1);", method_error_message("list.__add", 1, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].__add([], true);", method_error_message("list.__add", 1, "list", "bool"), $YASL_TYPE_ERROR);
assert_output("echo [] + true;", method_error_message("list.__add", 1, "list", "bool"), $YASL_TYPE_ERROR);
assert_output("echo []->__add(true);", method_error_message("list.__add", 1, "list", "bool"), $YASL_TYPE_ERROR);
assert_output("echo [].extend(1, []);", method_error_message("list.extend", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].extend(1, 1.0);", method_error_message("list.extend", 1, "list", "float"), $YASL_TYPE_ERROR);
assert_output("echo [].extend([], true);", method_error_message("list.extend", 1, "list", "bool"), $YASL_TYPE_ERROR);
assert_output("echo []->extend(1);", method_error_message("list.extend", 1, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].pop(1);", method_error_message("list.pop", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo []->pop();",
    $RED . "ValueError: list.pop expected list of length greater then 0 as arg 0.\n" . $END, $YASL_VALUE_ERROR);
# TODO: __get
# TODO: __set
assert_output("echo [].tostr(1);", method_error_message("list.tostr", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].search(1, .str);", method_error_message("list.search", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].reverse(1);", method_error_message("list.reverse", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].clear(1);", method_error_message("list.clear", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].join(1, .str);", method_error_message("list.join", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].join(1, true);", method_error_message("list.join", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo [].join([], 1);", method_error_message("list.join", 1, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo []->join(1);", method_error_message("list.join", 1, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo [].sort(1);", method_error_message("list.sort", 0, "list", "int"), $YASL_TYPE_ERROR);
assert_output("echo [1, .a]->sort();", $RED . "ValueError: list.sort expected a list of all numbers or all strings.\n" . $END, $YASL_VALUE_ERROR);

# Type Error (Str Methods)
assert_output("echo ''.tofloat(1);", method_error_message("str.tofloat", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.toint(1);", method_error_message("str.toint", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.isalnum(1);", method_error_message("str.isalnum", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.isal(1);", method_error_message("str.isal", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.isnum(1);", method_error_message("str.isnum", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.isspace(1);", method_error_message("str.isspace", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.tobool(1);", method_error_message("str.tobool", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.tostr(1);", method_error_message("str.tostr", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.toupper(1);", method_error_message("str.toupper", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.tolower(1);", method_error_message("str.tolower", 0, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.startswith(1, true);", method_error_message("str.startswith", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.startswith(1, .true);", method_error_message("str.startswith", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.startswith(.str, true);", method_error_message("str.startswith", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->startswith(1);", method_error_message("str.startswith", 1, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.endswith(1, true);", method_error_message("str.endswith", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.endswith(1, .true);", method_error_message("str.endswith", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.endswith(.str, true);", method_error_message("str.endswith", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->endswith(1);", method_error_message("str.endswith", 1, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.replace(1, true, 1.0);", method_error_message("str.replace", 2, "str", "float"), $YASL_TYPE_ERROR);
assert_output("echo ''.replace(1, true, .str);", method_error_message("str.replace", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.replace(1, .true, .str);", method_error_message("str.replace", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.replace(1, .true, true);", method_error_message("str.replace", 2, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.replace(.tr, true, .str);", method_error_message("str.replace", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.replace(.tr, .true, 1);", method_error_message("str.replace", 2, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.replace(.tr, true, 1);", method_error_message("str.replace", 2, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.replace(.tr, '', .sad);",
    $RED . "ValueError: str.replace expected a str of length greater than 0 as arg 1.\n". $END, $YASL_VALUE_ERROR);

assert_output("echo ''.search(1, true);", method_error_message("str.search", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.search(1, .true);", method_error_message("str.search", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.search(.str, true);", method_error_message("str.search", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->search(1);", method_error_message("str.search", 1, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.count(1, true);", method_error_message("str.count", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.count(1, .true);", method_error_message("str.count", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.count(.str, true);", method_error_message("str.count", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->count(1);", method_error_message("str.count", 1, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.split(1, true);", method_error_message("str.split", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.split(1, .true);", method_error_message("str.split", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.split(.str, true);", method_error_message("str.split", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->split(1);", method_error_message("str.split", 1, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo 'wasd'->split('');",
    $RED . "ValueError: str.split expected a str of length greater than 0 as arg 1.\n" . $END, $YASL_VALUE_ERROR);

assert_output("echo ''.ltrim(1, true);", method_error_message("str.ltrim", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.ltrim(1, .true);", method_error_message("str.ltrim", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.ltrim(.str, true);", method_error_message("str.ltrim", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->ltrim(1);", method_error_message("str.ltrim", 1, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.rtrim(1, true);", method_error_message("str.rtrim", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.rtrim(1, .true);", method_error_message("str.rtrim", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.rtrim(.str, true);", method_error_message("str.rtrim", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->rtrim(1);", method_error_message("str.rtrim", 1, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.trim(1, true);", method_error_message("str.trim", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.trim(1, .true);", method_error_message("str.trim", 0, "str", "int"), $YASL_TYPE_ERROR);
assert_output("echo ''.trim(.str, true);", method_error_message("str.trim", 1, "str", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->trim(1);", method_error_message("str.trim", 1, "str", "int"), $YASL_TYPE_ERROR);

assert_output("echo ''.rep(1, true);", method_error_message("str.rep", 1, "int", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''.rep(1.0, 1);", method_error_message("str.rep", 0, "str", "float"), $YASL_TYPE_ERROR);
assert_output("echo ''.rep(.str, true);", method_error_message("str.rep", 1, "int", "bool"), $YASL_TYPE_ERROR);
assert_output("echo ''->rep(true);", method_error_message("str.rep", 1, "int", "bool"), $YASL_TYPE_ERROR);
assert_output("echo 'as'->rep(-1);",
    $RED . "ValueError: str.rep expected non-negative int as arg 1.\n" . $END, $YASL_VALUE_ERROR);

# Type Error (Table Methods)
assert_output("echo {}.remove(1, 2.0);", method_error_message("table.remove", 0, "table", "int"), $YASL_TYPE_ERROR);
assert_output("echo {}.keys(1);", method_error_message("table.keys", 0, "table", "int"), $YASL_TYPE_ERROR);
assert_output("echo {}.values(1);", method_error_message("table.values", 0, "table", "int"), $YASL_TYPE_ERROR);
assert_output("echo {}.copy(1);", method_error_message("table.copy", 0, "table", "int"), $YASL_TYPE_ERROR);
assert_output("echo {}.tostr(1);", method_error_message("table.tostr", 0, "table", "int"), $YASL_TYPE_ERROR);
# TODO: __get
# TODO: __set
assert_output("echo {}.clear(1);", method_error_message("table.clear", 0, "table", "int"), $YASL_TYPE_ERROR);

# Type Error (Undef Methods)
assert_output("echo undef.tostr(1);", method_error_message("undef.tostr", 0, "undef", "int"), $YASL_TYPE_ERROR);

#DivisionByZeroError
assert_output(qq"echo 1 // 0;", $RED . "DivisionByZeroError\n" . $END, $YASL_DIVISION_BY_ZERO_ERROR);
assert_output(qq"echo 1 % 0;", $RED . "DivisionByZeroError\n" . $END, $YASL_DIVISION_BY_ZERO_ERROR);

exit $__VM_TESTS_FAILED__;
