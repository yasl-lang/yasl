#pragma once

#include "color.h"

#define MSG_SYNTAX_ERROR "SyntaxError: "
#define MSG_TYPE_ERROR "TypeError: "

#define YASL_PRINT_ERROR(fmt, ...) printf(K_RED fmt K_END, __VA_ARGS__)
#define YASL_PRINT_ERROR_SYNTAX(fmt, ...) YASL_PRINT_ERROR(MSG_SYNTAX_ERROR fmt, __VA_ARGS__)
#define YASL_PRINT_ERROR_TYPE(fmt, ...) YASL_PRINT_ERROR(MSG_TYPE_ERROR fmt, __VA_ARGS__)

#define YASL_PRINT_ERROR_UNDECLARED_VAR(name, line) YASL_PRINT_ERROR_SYNTAX("Undeclared variable %s (line %zd).\n", name, line)
#define YASL_PRINT_ERROR_CONSTANT(name, line) YASL_PRINT_ERROR_SYNTAX("Cannot assign to constant %s (line %zd).", name, line)
#define YASL_PRINT_ERROR_DIVIDE_BY_ZERO() printf(K_RED "DivisionByZeroError" K_END)
