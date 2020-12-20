#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/lexer.h"
#include "opcode.h"
#include "yasl_include.h"

#define NUM_FAILED __YATS_TESTS_FAILED__

#define _YATS_MANGLE_NAME(name) _YATS__##name

#define TEST(name) int _YATS_MANGLE_NAME(name)(void)
#define RUN(name) __YATS_TESTS_FAILED__ |= _YATS_MANGLE_NAME(name)()

#define SETUP_YATS() \
	static int __YATS_TESTS_FAILED__ = 0

#define TEST_FAILED() __YATS_TESTS_FAILED__ = 1

// change to true to print out all passing tests as well.
#define SHOW_PASSING false

#define ASSERT_BC_EQ(left, right, sr) do {\
	if (sizeof(left) == sr && !memcmp(left, right, sizeof(left))) {\
		if (SHOW_PASSING) printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);\
	} else {\
		printf(K_RED "assert failed in %s: line %d (%s)" K_END "\n", __func__, __LINE__, __FILE__);\
		puts("expected: ");\
		for (size_t i = 0; i < sizeof(left); i++) printf(i < sr && (left)[i] == (right)[i] ? K_GRN "%02x " : K_RED "%02x ", (left)[i] & 0xFF);\
		printf(K_END "\n");\
		puts("actual: ");\
		for (size_t i = 0; i < sr; i++) printf(i < sizeof(left) && (left)[i] == (right)[i] ? K_GRN "%02x " : K_RED "%02x ", (right)[i] & 0xFF);\
		printf(K_END "\n");\
		TEST_FAILED();\
	}\
} while(0)

#define ASSERT_GEN_BC_EQ(expected, fc) do{\
	remove("dump.yb");\
	setup_compiler(fc);\
	FILE *file = fopen("dump.yb", "rb");\
	int64_t size = getsize(file);\
	unsigned char *actual = (unsigned char *)malloc(size);\
	size_t read = fread(actual, sizeof(char), size, file);\
	(void) read;\
	ASSERT_BC_EQ(expected, actual, (size_t)size);\
	fclose(file);\
	free(actual);\
} while(0)

#define ASSERT_EQ(left, right) do {\
	if ((left) != (right)) {\
		printf(K_RED "assert failed in %s (in %s): line %d: `%s` =/= `%s`" K_END "\n", __FILE__, __func__, __LINE__, #left, #right);\
		TEST_FAILED();\
	} else {\
		if (SHOW_PASSING) printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);\
	}\
} while(0)

#define ASSERT_STR_EQ(left, right, len) do {\
	if (memcmp(left, right, len) != 0) {\
	printf(K_RED "assert failed in %s (in %s): line %d: `%s` =/= `%s`" K_END "\n", __FILE__, __func__, __LINE__, left, right);\
		TEST_FAILED();\
	} else {\
		if (SHOW_PASSING) printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);\
	}\
} while(0)

#define ASSERT_SUCCESS(expr) ASSERT_EQ((expr), YASL_SUCCESS)
#define ASSERT(expr) ASSERT_EQ((expr), true)

struct Lexer setup_lexer(const char *file_contents);
unsigned char *setup_compiler(const char *file_contents);
int64_t getsize(FILE *file);
