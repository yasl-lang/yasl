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

# Integer Methods
assert_output("echo 2->tofloat64()\n", "2.0\n", 0);
assert_output("echo 5->tostr()\n", "5\n", 0);


# Float Methods
assert_output("echo 2.1->toint64()\n", "2\n", 0);
assert_output("echo 5.7->tostr()\n", "5.7\n", 0);

# Boolean Methods
assert_output("echo true->tostr()\n", "true\n", 0);
assert_output("echo false->tostr()\n", "false\n", 0);


# String Methods
assert_output("echo 'yasl'->tobool()\n", "true\n", 0);
assert_output("echo ''->tobool()\n", "false\n", 0);
assert_output("echo 'yasl'->tostr()\n", "yasl\n", 0);
assert_output("echo 'yasl'->toupper()\n", "YASL\n", 0);
assert_output("echo 'Yasl'->toupper()\n", "YASL\n", 0);
assert_output("echo 'Yasl'->tolower()\n", "yasl\n", 0);
assert_output("echo 'Yasl12_'->isalnum()\n", "false\n", 0);
assert_output("echo 'YASL1223121321'->isalnum()\n", "true\n", 0);
assert_output("echo 'YASL1223121321'->isal()\n", "false\n", 0);
assert_output("echo 'YASLDSADASDAS'->isal()\n", "true\n", 0);
assert_output("echo 'YASLDSADASDAS'->isnum()\n", "false\n", 0);
assert_output("echo '12341213421'->isnum()\n", "true\n", 0);
assert_output("echo '12341213421'->isspace()\n", "false\n", 0);
assert_output("echo '  '->isspace()\n", "true\n", 0);
assert_output("echo 'YASL'->startswith('YA')\n", "true\n", 0);
assert_output("echo 'YASL'->startswith('A')\n", "false\n", 0);
assert_output("echo 'YASL'->endswith('ASL')\n", "true\n", 0);
assert_output("echo 'YASL'->endswith('YA')\n", "false\n", 0);
assert_output("echo 'YASL'->replace('T', 'S')\n", "YASL\n", 0);
assert_output("echo 'YASL'->replace('SL', '')\n", "YA\n", 0);
assert_output("echo 'YASL'->search('A')\n", "1\n", 0);
assert_output("echo 'YASL'->search('0')\n", "undef\n", 0);
assert_output("echo len 'the quick brown fox'->split(' ')\n", "4\n", 0); 
assert_output("echo len 'the quick brown fox'->split('0')\n", "1\n", 0);
assert_output("echo 'YAY'->ltrim('Y')\n", "AY\n", 0);
assert_output("echo 'YYYYAY'->ltrim('Y')\n", "AY\n", 0);
assert_output("echo 'YAY'->ltrim('A')\n", "YAY\n", 0);
assert_output("echo 'YAY'->rtrim('Y')\n", "YA\n", 0);
assert_output("echo 'YAYYYY'->rtrim('Y')\n", "YA\n", 0);
assert_output("echo 'YAY'->rtrim('A')\n", "YAY\n", 0);
assert_output("echo 'YAY'->trim('Y')\n", "A\n", 0);
assert_output("echo 'YAY'->trim('A')\n", "YAY\n", 0);
assert_output("echo 'YYAYYY'->trim('Y')\n", "A\n", 0);
assert_output("echo 'YASL'->__get(3)\n", "L\n", 0); 
assert_output("echo 'YASL'->__get(-1)\n", "L\n", 0);

# List Methods
assert_output("let x = [0]\n" .
              "x->push(1)\n" . 
              "for let e <- x { echo e; }\n", "0\n1\n", 0);
assert_output("let x = [1, 2, 3]\n" .
           "echo x->pop()\n" .
           "for let e <- x { echo e; }\n", "3\n1\n2\n", 0);
assert_output("let x = [1, 2, 3]\n" .
              "x->extend([4, 5, 6])\n" .
              "for let e <- x { echo e; }\n", "1\n2\n3\n4\n5\n6\n", 0);
assert_output("let x = [1, 2, 3]\n" .
              "echo x->search(4)\n", "undef\n", 0);
assert_output("let x = [1, 2, 4]\n" .
              "echo x->search(4)\n", "2\n", 0);
assert_output("let x = [1, 2, 3, 4, 5]\n" .
              "x->reverse()\n" .
              "for let e <- x { echo e; }\n", "5\n4\n3\n2\n1\n", 0);
 
# Table Methods
assert_output("let x = {1:'one', 2:'two', 3:'three'}\n" .
              "for let e <- x->keys() { echo e; }\n", "3\n1\n2\n", 0);
assert_output("let x = {1:'one', 2:'two', 3:'three'}\n" .
              "for let e <- x->values() { echo e; }\n", "three\none\ntwo\n", 0);




















exit $__YASL_TESTS_FAILED__;
