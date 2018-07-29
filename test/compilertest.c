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

#define ASSERT_GEN_BC_EQ(expected, fc) do{\
    remove("dump.yb");\
    setup_compiler(fc);\
    FILE *file = fopen("dump.yb", "rb");\
    int64_t size = getsize(file);\
    unsigned char actual[size];\
    fread(actual, sizeof(char), size, file);\
    ASSERT_BC_EQ(expected, actual);\
    fclose(file);\
} while(0)

static int64_t getsize(FILE *file) {
    fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

// NOTE: these tests depend on the endianess of the system, so they may fail on big endian systems.

/// Literals
////////////////////////////////////////////////////////////////////////////////

static void test_undef() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            NCONST,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "undef;");
}

static void test_true() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "true;");
}

static void test_false() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_F,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "false;");
}

static void test_bin() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "0b11;");
}

static void test_oct() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "0o12;");
}

static void test_dec() {
    unsigned char expected[]  = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "10;");
}

static void test_hex() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "0x10;");
}

int compilertest() {
    test_undef();
    test_true();
    test_false();
    test_bin();
    test_oct();
    test_dec();
    test_hex();

    return __YASL_TESTS_FAILED__;
}