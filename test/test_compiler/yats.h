#pragma once

#include "lexer.h"
#include "yasl_include.h"
#include "opcode.h"

#define SETUP_YATS() \
    static int __YASL_TESTS_FAILED__ = 0


#define ASSERT_BC_EQ(left, right) do {\
    if (sizeof(left) == sizeof(right) && !memcmp(left, right, sizeof(left))) {\
        /*printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);*/\
    } else {\
        printf(K_RED "assert failed in %s: line %d (%s)" K_END "\n", __func__, __LINE__, __FILE__);\
        puts("expected: ");\
        for (size_t i = 0; i < sizeof(left); i++) printf(i < sizeof(right) && (left)[i] == (right)[i] ? K_GRN "%02x " : K_RED "%02x ", (left)[i] & 0xFF);\
        printf(K_END "\n");\
        puts("actual: ");\
        for (size_t i = 0; i < sizeof(right); i++) printf(i < sizeof(left) && (left)[i] == (right)[i] ? K_GRN "%02x " : K_RED "%02x ", (right)[i] & 0xFF);\
        printf(K_END "\n");\
        __YASL_TESTS_FAILED__ = 1;\
    }\
} while(0)

#define ASSERT_GEN_BC_EQ(expected, fc) do{\
    remove("dump.yb");\
    unsigned char *bytecode = setup_compiler(fc);\
    FILE *file = fopen("dump.yb", "rb");\
    int64_t size = getsize(file);\
    unsigned char actual[size];\
    fread(actual, sizeof(char), size, file);\
    ASSERT_BC_EQ(expected, actual);\
    fclose(file);\
} while(0)

Lexer *setup_lexer(char *file_contents);
unsigned char *setup_compiler(char *file_contents);
int64_t getsize(FILE *file);