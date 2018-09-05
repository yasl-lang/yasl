require "yasl_test.pl";

# Unary Operators
assert_output("print -10\n", "-10\n", 0);
assert_output("print \@'YASL'\n", "4\n", 0);
assert_output("print !true\n", "false\n", 0);
assert_output("print !false\n", "true\n", 0);
assert_output("print ^0b00000000\n", "-1\n", 0);

# Binary Operators
assert_output("print 2 ** 4\n", "16\n", 0);
assert_output("print 2 ** -1\n", "0.5\n", 0);
assert_output("print -2 ** -1\n", "-0.5\n", 0);

assert_output("print 5 * 6\n", "30\n", 0);
assert_output("print 5 / 2\n", "2.5\n", 0);
assert_output("print 6 // 2\n", "3\n", 0);
assert_output("print 5 % 2\n", "1\n", 0);

assert_output("print 10 + 12\n", "22\n", 0);
assert_output("print 3.5 - 2\n", "1.5\n", 0);

assert_output("print 1 || 2\n", "1\n", 0);
assert_output("print 1 && 2\n", "2\n", 0);

# String Methods
assert_output("print 'yasl'::tobool()\n", "true\n", 0);
assert_output("print ''::tobool()\n", "false\n", 0);
assert_output("print 'yasl'::tostr()\n", "yasl\n", 0);
assert_output("print 'yasl'::toupper()\n", "YASL\n", 0);
assert_output("print 'Yasl'::tolower()\n", "yasl\n", 0);
assert_output("print 'Yasl12_'::isalnum()\n", "false\n", 0);
assert_output("print 'YASL1223121321'::isalnum()\n", "true\n", 0);
assert_output("print 'YASL1223121321'::isal()\n", "false\n", 0);
assert_output("print 'YASLDSADASDAS'::isal()\n", "true\n", 0);
assert_output("print 'YASLDSADASDAS'::isnum()\n", "false\n", 0);
assert_output("print '12341213421'::isnum()\n", "true\n", 0);
assert_output("print '12341213421'::isspace()\n", "false\n", 0);
assert_output("print '  '::isspace()\n", "true\n", 0);

exit $__YASL_TESTS_FAILED__;
