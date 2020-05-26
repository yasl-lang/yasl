#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yasl.h"
#include "yasl_aux.h"

#define MAX_FILE_NAME_LEN 1024

#define ERROR_TEST(errors) \
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
	fread(expected_output, 1, size, f);\
	expected_output[size] = '\0';\
	S = YASL_newstate(errors[i]);\
	YASLX_decllibs(S);\
	YASL_setprinterr_tostr(S);\
	YASL_execute(S);\
	YASL_loadprinterr(S);\
	char *actual_output = YASL_peekcstr(S);\
	if (!!strcmp(expected_output, actual_output)) {\
		fprintf(stderr, "test for %s failed (expected %s, got %s).\n", errors[i], expected_output, actual_output);\
		failed++;\
	}\
	free(expected_output);\
	YASL_delstate(S);\
}

int main(void) {
	int failed = 0;
#include "outputs.inl"
#include "assert_errors.inl"
	char buffer[MAX_FILE_NAME_LEN];
	struct YASL_State *S;
	for (size_t i = 0; i < sizeof(outputs) / sizeof(char *); i++) {
		if (strlen(outputs[i]) + 5 > MAX_FILE_NAME_LEN) {
			printf("file name too large: %s\n", outputs[i]);
			exit(1);
		}
		strcpy(buffer, outputs[i]);
		strcpy(buffer + strlen(outputs[i]), ".out");
		FILE *f = fopen(buffer, "r");
		fseek(f, 0, SEEK_END);
		size_t size = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *expected_output = (char *)malloc(size + 1);
		fread(expected_output, 1, size, f);
		expected_output[size] = '\0';

		S = YASL_newstate(outputs[i]);
		YASLX_decllibs(S);
		YASL_setprintout_tostr(S);
		YASL_execute(S);
		YASL_loadprintout(S);
		char *actual_output = YASL_peekcstr(S);
		if (!!strcmp(expected_output, actual_output)) {
			fprintf(stderr, "test for %s failed.\n", outputs[i]);
			failed++;
		}
		free(expected_output);
		YASL_delstate(S);
	}

	ERROR_TEST(assert_errors);

	return failed;
}
