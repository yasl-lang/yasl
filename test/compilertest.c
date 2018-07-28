#include "compiler/ast/ast.h"
#include "compiler/parser/parser.h"
#include <compiler/compiler/compiler.h>
#include <color.h>
#include "yats.h"
#include "compilertest.h"

void setup_compiler(char *file_contents) {
    Parser *parser = parser_new(setup_lexer(file_contents));
    Compiler *compiler = compiler_new(parser, "dump.yb");
    compile(compiler);
    compiler_del(compiler);
}

SETUP_YATS();

#define ASSERT_BC_EQ(left, right) do {\
    if (sizeof(left) == sizeof(right) && !memcmp(left, right, sizeof(left))) {\
        /*printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);*/\
    } else {\
        printf(K_RED "assert failed in %s: line %d" K_END "\n", __func__, __LINE__);\
        puts("expected: ");\
        for (int i = 0; i < sizeof(left); i++) printf(i < sizeof(right) && (left)[i] == (right)[i] ? K_GRN "%02x " : K_RED "%02x ", (left)[i] & 0xFF);\
        printf(K_END "\n");\
        puts("actual: ");\
        for (int i = 0; i < sizeof(right); i++) printf(i < sizeof(left) && (left)[i] == (right)[i] ? K_GRN "%02x " : K_RED "%02x ", (right)[i] & 0xFF);\
        printf(K_END "\n");\
        __YASL_TESTS_FAILED__ = 1;\
    }\
} while(0)

#define ASSERT_GEN_BC_EQ(expected) do{\
    FILE *file = fopen("dump.yb", "rb");\
    int64_t size = getsize(file);\
    unsigned char actual[size];\
    fread(actual, sizeof(char), size, file);\
    ASSERT_BC_EQ(expected, actual);\
} while(0)

static int64_t getsize(FILE *file) {
    fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}


static void test_print() {
    unsigned char expected[]  = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            PRINT,
            HALT
    };
    setup_compiler("print 10;");
    ASSERT_GEN_BC_EQ(expected);
}

int compilertest() {
    test_print();

    return __YASL_TESTS_FAILED__;
}