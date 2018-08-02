require "yasl_test.pl";



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
