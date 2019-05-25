#include "yasl-std-io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define YASL_FILE (-3)

int YASL_io_open(struct YASL_State *S) {
	struct YASL_Object *mode = YASL_popobject(S);
	const char *mode_str;
	if (YASL_isundef(mode) == YASL_SUCCESS) {
		mode_str = "r";
	} else if (YASL_isstring(mode) == YASL_SUCCESS) {
		mode_str = YASL_getcstring(mode);
	} else {
		return -1;
	}

	struct YASL_Object *filename = YASL_popobject(S);
	if (YASL_isstring(filename) != YASL_SUCCESS) {
		return -1;
	}
	char *filename_str = YASL_getcstring(filename);

	size_t mode_len = strlen(mode_str);

	if (mode_len > 2 || mode_len < 1 || (mode_len == 2 && mode_str[1] != '+')) {
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
		case 'r':f = fopen(filename_str, "r+");
			break;
		case 'w':f = fopen(filename_str, "w+");
			break;
		case 'a':f = fopen(filename_str, "a+");
			break;
		default:
			// invalid mode;
			free(filename_str);
			return -1;
		}
	}
	struct YASL_Object *ud = f ? YASL_UserData(f, YASL_FILE, NULL, NULL) : YASL_Undef();
	YASL_pushobject(S, ud);
	free(filename_str);
	return 0;
}

int YASL_io_read(struct YASL_State *S) {
	struct YASL_Object *mode = YASL_popobject(S);
	const char *mode_str;

	if (YASL_isundef(mode) == YASL_SUCCESS) {
		mode_str = "a";
	} else if (YASL_isstring(mode) == YASL_SUCCESS) {
		mode_str = YASL_getcstring(mode);
	} else {
		return -1;
	}

	struct YASL_Object *file = YASL_popobject(S);
	FILE *f;


	if (YASL_isuserdata(file, YASL_FILE) == YASL_SUCCESS) {
		f = (FILE *)YASL_UserData_getdata(file);
	} else {
		return -1;
	}

	size_t mode_len = strlen(mode_str);

	if (mode_len != 1) {
		return -1;
	}

	switch (mode_str[0]) {
	case 'a': {
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *string = (char *)malloc(fsize + 1);
		fread(string, fsize, 1, f);
		string[fsize] = '\0';
		YASL_pushobject(S, YASL_CString(string)); }
		return 0;
	default:
		return -1;
	}
}

int YASL_io_write(struct YASL_State *S) {
	struct YASL_Object *obj = YASL_popobject(S);

	if (YASL_isstring(obj) != YASL_SUCCESS) {
		return -1;
	}

	struct YASL_Object *file = YASL_popobject(S);
	FILE *f;


	if (YASL_isuserdata(file, YASL_FILE) == YASL_SUCCESS) {
		f = (FILE *)YASL_UserData_getdata(file);
	} else {
		return -1;
	}

	char *str = YASL_getcstring(obj);
	size_t len = YASL_getstringlen(obj);

	size_t write_len = fwrite(str, 1, len, f);

	YASL_pushinteger(S, write_len);
	free(str);
	return YASL_SUCCESS;
}

int YASL_io_flush(struct YASL_State *S) {
	struct YASL_Object *file = YASL_popobject(S);
	FILE *f;


	if (YASL_isuserdata(file, YASL_FILE) == YASL_SUCCESS) {
		f = (FILE *)YASL_UserData_getdata(file);
	} else {
		return -1;
	}

	int success = fflush(f);

	YASL_pushboolean(S, success == 0);

	return YASL_SUCCESS;
}

int YASL_load_io(struct YASL_State *S) {
	struct YASL_Object *io = YASL_Table();

	struct YASL_Object *open_str = YASL_LiteralString("open");
	struct YASL_Object *open_fn = YASL_CFunction(YASL_io_open, 2);

	YASL_Table_set(io, open_str, open_fn);

	struct YASL_Object *read_str = YASL_LiteralString("read");
	struct YASL_Object *read_fn = YASL_CFunction(YASL_io_read, 2);

	YASL_Table_set(io, read_str, read_fn);

	struct YASL_Object *write_str = YASL_LiteralString("write");
	struct YASL_Object *write_fn = YASL_CFunction(YASL_io_write, 2);

	YASL_Table_set(io, write_str, write_fn);

	struct YASL_Object *flush_str = YASL_LiteralString("flush");
	struct YASL_Object *flush_fn = YASL_CFunction(YASL_io_flush, 1);

	YASL_Table_set(io, flush_str, flush_fn);
/*
	struct YASL_Object *stdin_str = YASL_CString("stdin");
	struct YASL_Object *stdin_file = YASL_UserData(stdin, YASL_FILE, NULL);
	struct YASL_Object *stdout_str = YASL_CString("stdout");
	struct YASL_Object *stdout_file = YASL_UserData(stdout, YASL_FILE, NULL);
	struct YASL_Object *stderr_str = YASL_CString("stderr");
	struct YASL_Object *stderr_file = YASL_UserData(stderr, YASL_FILE, NULL);

	YASL_Table_set(io, stdin_str, stdin_file);
	YASL_Table_set(io, stdout_str, stdout_file);
	YASL_Table_set(io, stderr_str, stderr_file);
*/
	YASL_declglobal(S, "io");
	YASL_pushobject(S, io);
	YASL_setglobal(S, "io");

	free(open_str);
	free(open_fn);
	free(read_str);
	free(read_fn);
	free(write_str);
	free(write_fn);
	free(flush_str);
	free(flush_fn);

/*
	free(stdin_str);
	free(stdin_file);
	free(stdout_str);
	free(stdout_file);
	free(stderr_str);
	free(stderr_file);
*/
	return YASL_SUCCESS;
}

