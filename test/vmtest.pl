require "yasl_test.pl";

# Unary Operators
assert_output("echo -10\n", "-10\n", 0);
assert_output("echo len'YASL'\n", "4\n", 0);
assert_output("echo !true\n", "false\n", 0);
assert_output("echo !false\n", "true\n", 0);
assert_output("echo ^0b00000000\n", "-1\n", 0);

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

# String Methods
assert_output("echo 'yasl'::tobool()\n", "true\n", 0);
assert_output("echo ''::tobool()\n", "false\n", 0);
assert_output("echo 'yasl'::tostr()\n", "yasl\n", 0);
assert_output("echo 'yasl'::toupper()\n", "YASL\n", 0);
assert_output("echo 'Yasl'::tolower()\n", "yasl\n", 0);
assert_output("echo 'Yasl12_'::isalnum()\n", "false\n", 0);
assert_output("echo 'YASL1223121321'::isalnum()\n", "true\n", 0);
assert_output("echo 'YASL1223121321'::isal()\n", "false\n", 0);
assert_output("echo 'YASLDSADASDAS'::isal()\n", "true\n", 0);
assert_output("echo 'YASLDSADASDAS'::isnum()\n", "false\n", 0);
assert_output("echo '12341213421'::isnum()\n", "true\n", 0);
assert_output("echo '12341213421'::isspace()\n", "false\n", 0);
assert_output("echo '  '::isspace()\n", "true\n", 0);

exit $__YASL_TESTS_FAILED__;
