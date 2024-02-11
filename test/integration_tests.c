#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yasl.h"
#include "yasl_aux.h"

#include <yasl_test.h>

#define MAX_FILE_NAME_LEN 1024

#define INPUT_TEST(inputs) \
for (size_t i = 0; i < sizeof(inputs) / sizeof(char *); i++) {\
	if (strlen(inputs[i]) + 5 > MAX_FILE_NAME_LEN) {\
		fprintf(stderr, "file name too large: %s\n", inputs[i]);\
		exit(1);\
	}\
	strcpy(buffer, inputs[i]);\
	strcpy(buffer + strlen(inputs[i]), ".out");\
	FILE *f = fopen(buffer, "r");\
	if (f == NULL) {\
		fprintf(stderr, "missing file: %s\n", buffer);\
		failed++;\
		continue;\
	}\
	fseek(f, 0, SEEK_END);\
	size_t size = ftell(f);\
	fseek(f, 0, SEEK_SET);\
	char *expected_output = (char *)malloc(size + 1);\
	size_t read = fread(expected_output, 1, size, f);\
	expected_output[read] = '\0';\
	S = YASL_newstate(inputs[i]);\
	YASLX_decllibs(S);\
	YASL_setprintout_tostr(S);\
	int status = YASL_execute(S);\
	YASL_loadprintout(S);\
	char *actual_output = YASL_peekcstr(S);\
	if (!!strcmp(expected_output, actual_output) || status != YASL_SUCCESS) {\
		fprintf(stderr, "test for %s failed (expected:\n`%s`, got:\n`%s`).\n", inputs[i], expected_output, actual_output);\
		failed++;\
	}\
	ran++;\
	free(actual_output);\
	free(expected_output);\
	YASL_delstate(S);\
}

#define ERROR_TEST(errors, error_code) \
for (size_t i = 0; i < sizeof(errors) / sizeof(char *); i++) {\
	if (strlen(errors[i]) + 5 > MAX_FILE_NAME_LEN) {\
		printf("file name too large: %s\n", errors[i]);\
		exit(1);\
	}\
	strcpy(buffer, errors[i]);\
	strcpy(buffer + strlen(errors[i]), ".err");\
	FILE *f = fopen(buffer, "r");\
	fseek(f, 0, SEEK_END);\
	size_t size = ftell(f);\
	fseek(f, 0, SEEK_SET);\
	char *expected_output = (char *)malloc(size + 1);\
	size_t read = fread(expected_output, 1, size, f);\
	expected_output[read] = '\0';\
	S = YASL_newstate(errors[i]);\
	YASLX_decllibs(S);\
	YASL_setprinterr_tostr(S);\
	int status = YASL_execute(S);\
	YASL_loadprinterr(S);\
	char *actual_output = YASL_peekcstr(S);\
	if (!!strcmp(expected_output, actual_output) || status != error_code) {\
		fprintf(stderr, "test for %s failed (expected:\n`%s`, got:\n`%s`).\n", errors[i], expected_output, actual_output);\
		failed++;\
	}\
	ran++;\
	free(actual_output);\
	free(expected_output);\
	YASL_delstate(S);\
}

int main(void) {
	int result = 0;
	int failed = 0;
	int ran = 0;
#include "inputs.inl"
#include "assert_errors.inl"
#include "stackoverflow_errors.inl"
#include "type_errors.inl"
#include "value_errors.inl"
#include "divisionbyzero_errors.inl"
#include "error_errors.inl"
#include "syntax_errors.inl"

	char buffer[MAX_FILE_NAME_LEN];
	struct YASL_State *S;

	INPUT_TEST(inputs);
	ERROR_TEST(assert_errors, YASL_ASSERT_ERROR);
	ERROR_TEST(stackoverflow_errors, YASL_STACK_OVERFLOW_ERROR);
	ERROR_TEST(error_errors, YASL_ERROR);
	ERROR_TEST(type_errors, YASL_TYPE_ERROR);
	ERROR_TEST(value_errors, YASL_VALUE_ERROR);
	ERROR_TEST(divisionbyzero_errors, YASL_DIVIDE_BY_ZERO_ERROR);
	ERROR_TEST(syntax_errors, YASL_SYNTAX_ERROR);

	result = result || failed;
	printf("Failed %d (/%d) script tests.\n", failed, ran);

	failed = unit_tests();

	result = result || failed;
	printf("Failed %d unit tests.\n", failed);

	printf("Tests exited with code %d.\n", result);
	return result;
}
