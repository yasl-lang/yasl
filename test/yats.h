#pragma once

#include "lexer.h"

#define SETUP_YATS() \
    static int __YASL_TESTS_FAILED__ = 0

Lexer *setup_lexer(char *file_contents);