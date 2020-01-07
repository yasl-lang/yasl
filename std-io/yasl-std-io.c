#include "yasl-std-io.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data-structures/YASL_Table.h"

static struct YASL_Table *mt;

// TODO: fix mem leak in here.
static int YASL_io_open(struct YASL_State *S) {
	const char *mode_str;
	if (YASL_top_isundef(S)) {
		mode_str = "r";
	} else if (YASL_top_isstring(S)) {
		mode_str = YASL_top_peekcstring(S);
	} else {
		// TODO error message
		return YASL_TYPE_ERROR;
	}
	YASL_pop(S);

	if (!YASL_top_isstring(S)) {
		// TODO error message
		return YASL_TYPE_ERROR;
	}

	char *filename_str = YASL_top_peekcstring(S);
	YASL_pop(S);

	size_t mode_len = strlen(mode_str);

	if (mode_len > 2 || mode_len < 1 || (mode_len == 2 && mode_str[1] != '+')) {
		// TODO error message
		return -1;
	}

	char mode_char = mode_str[0];

	FILE *f = 0;
	if (mode_len == 1) {
		switch (mode_char) {
		case 'r':f = fopen(filename_str, "r");
			break;
		case 'w':f = fopen(filename_str, "w");
			break;
		case 'a':f = fopen(filename_str, "a");
			break;
		default:
			// invalid mode;
			free(filename_str);
			return -1;
		}
	}
	if (mode_len == 2) {
		switch (mode_char) {
		case 'r':
			f = fopen(filename_str, "r+");
			break;
		case 'w':
			f = fopen(filename_str, "w+");
			break;
		case 'a':
			f = fopen(filename_str, "a+");
			break;
		default:
			// invalid mode;
			free(filename_str);
			// TODO error message
			return -1;
		}
	}
	if (f) {
		YASL_pushuserdata(S, f, T_FILE, mt, NULL);
	} else {
		YASL_pushundef(S);
	}
	free(filename_str);
	return YASL_SUCCESS;
}

static int YASL_io_read(struct YASL_State *S) {
	char *mode_str;

	if (YASL_top_isundef(S)) {
		mode_str = (char *)malloc(2);
		mode_str[0] = 'l';
		mode_str[1] = '\0';
	} else if (YASL_top_isstring(S)) {
		mode_str = YASL_top_peekcstring(S);
	} else {
		// TODO error message
		return YASL_TYPE_ERROR;
	}
	YASL_pop(S);

	if (!YASL_top_isuserdata(S, T_FILE)) {
		// TODO error message
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_top_popuserdata(S);

	size_t mode_len = strlen(mode_str);

	if (mode_len != 1) {
		free(mode_str);
		return -1;
	}

	switch (mode_str[0]) {
	case 'a': {
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *string = (char *)malloc(fsize);
		fread(string, fsize, 1, f);
		YASL_pushstring(S, string, fsize); }
		free(mode_str);
		return YASL_SUCCESS;
	case 'l': {
		size_t size = 16;
		char *string = (char *)malloc(size);
		size_t i = 0;
		int c;

		while ( (c = fgetc(f)) != EOF && c != '\n') {
			if (i == size) {
				size *= 2;
				string = (char *)realloc(string, size);
			}
			string[i++] = (char) c;
		}
		YASL_pushstring(S, string, i);
		free(mode_str);
		return YASL_SUCCESS;
	}
	default:
		free(mode_str);
		return -1;
	}
}

static int YASL_io_write(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		// TODO error message
		return YASL_TYPE_ERROR;
	}
	char *str = YASL_top_peekcstring(S);
	YASL_pop(S);

	if (!YASL_top_isuserdata(S, T_FILE)) {
		// TODO error message
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_top_popuserdata(S);

	size_t len = strlen(str);

	size_t write_len = fwrite(str, 1, len, f);

	YASL_pushinteger(S, write_len);
	free(str);
	return YASL_SUCCESS;
}

static int YASL_io_flush(struct YASL_State *S) {
	if (!YASL_top_isuserdata(S, T_FILE)) {
		// TODO error message
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_top_popuserdata(S);

	int success = fflush(f);

	YASL_pushboolean(S, success == 0);
	return YASL_SUCCESS;
}

int YASL_load_io(struct YASL_State *S) {
	if (!mt) {
		mt = YASL_Table_new();
		YASL_Table_insert(mt, YASL_STR(YASL_String_new_sized(strlen("read"), "read")),
				  YASL_CFN(YASL_io_read, 2));
		YASL_Table_insert(mt, YASL_STR(YASL_String_new_sized(strlen("write"), "write")),
				  YASL_CFN(YASL_io_write, 2));
		YASL_Table_insert(mt, YASL_STR(YASL_String_new_sized(strlen("flush"), "flush")),
				  YASL_CFN(YASL_io_flush, 1));
	}

	YASL_declglobal(S, "io");
	YASL_pushtable(S);
	YASL_setglobal(S, "io");

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "open");
	YASL_pushcfunction(S, YASL_io_open, 2);
	YASL_settable(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "read");
	YASL_pushcfunction(S, YASL_io_read, 2);
	YASL_settable(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "write");
	YASL_pushcfunction(S, YASL_io_write, 2);
	YASL_settable(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "flush");
	YASL_pushcfunction(S, YASL_io_flush, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "stdin");
	YASL_pushuserdata(S, stdin, T_FILE, mt, NULL);
	YASL_settable(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "stdout");
	YASL_pushuserdata(S, stdout, T_FILE, mt, NULL);
	YASL_settable(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "stderr");
	YASL_pushuserdata(S, stderr, T_FILE, mt, NULL);
	YASL_settable(S);

	return YASL_SUCCESS;
}

