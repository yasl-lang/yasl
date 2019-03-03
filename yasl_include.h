#pragma once

#define K_END "\x1B[0m"
#define K_RED "\x1B[31m"
#define K_GRN "\x1B[32m"
#define K_YEL "\x1B[33m"
#define K_BLU "\x1B[34m"
#define K_MAG "\x1B[35m"
#define K_CYN "\x1B[36m"
#define K_WHT "\x1B[37m"

#define MSG_SYNTAX_ERROR "SyntaxError: "
#define MSG_TYPE_ERROR "TypeError: "

#define YASL_PRINT_ERROR(fmt, ...) printf(K_RED fmt K_END, __VA_ARGS__)
#define YASL_PRINT_ERROR_SYNTAX(fmt, ...) YASL_PRINT_ERROR(MSG_SYNTAX_ERROR fmt, __VA_ARGS__)
#define YASL_PRINT_ERROR_TYPE(fmt, ...) YASL_PRINT_ERROR(MSG_TYPE_ERROR fmt, __VA_ARGS__)

#define YASL_PRINT_ERROR_UNDECLARED_VAR(name, line) YASL_PRINT_ERROR_SYNTAX("Undeclared variable %s (line %zd).\n", name, line)
#define YASL_PRINT_ERROR_CONSTANT(name, line) YASL_PRINT_ERROR_SYNTAX("Cannot assign to constant %s (line %zd).\n", name, line)
#define YASL_PRINT_ERROR_TOO_MANY_VAR(line) YASL_PRINT_ERROR("Too many variables in current scope (line %zd).\n", line)
#define YASL_PRINT_ERROR_DIVIDE_BY_ZERO() printf(K_RED "DivisionByZeroError\n" K_END)
