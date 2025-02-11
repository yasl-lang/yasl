#include "yasl-std-io.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data-structures/YASL_Table.h"
#include "VM.h"
#include "yasl_aux.h"

// what to prepend to method names in messages to user
#define FILE_PRE "io.file"

static const char *const FILE_NAME = "io.file";

static FILE *YASLX_checknfile(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnuserdata(S, FILE_NAME, pos)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, pos, FILE_NAME);
	}
	return (FILE *)YASL_peeknuserdata(S, pos);
}

static void close_file(struct YASL_State *S, void *ptr) {
	YASL_UNUSED(S);
	fclose((FILE *)ptr);
}

static int YASL_io_open(struct YASL_State *S) {
	char mode_str[3] = { '\0', '\0', '\0'};
	if (YASL_isundef(S)) {
		mode_str[0] = 'r';
	} else if (YASL_isnstr(S, 1)) {
		char *tmp = YASL_peekcstr(S);
		if (strlen(tmp) > 2 || strlen(tmp) < 1 || (strlen(tmp) == 2 && tmp[1] != '+')) {
			YASLX_print_err_value(S, "io.open was passed invalid mode: %*s.", (int)strlen(tmp), tmp);
			free(tmp);
			YASLX_throw_err_value(S);
		}
		mode_str[0] = tmp[0];
		mode_str[1] = tmp[1];
		free(tmp);
	} else {
		YASLX_print_and_throw_err_bad_arg_type_n(S, "io.open", 1, YASL_STR_NAME);
	}
	YASL_pop(S);

	if (!YASL_isnstr(S, 0)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, "io.open", 0, YASL_STR_NAME);
	}

	char *filename_str = YASL_peekcstr(S);

	char mode_char = mode_str[0];

	FILE *f = NULL;
	if (strlen(mode_str) == 1) {
		switch (mode_char) {
		case 'r':
			f = fopen(filename_str, "r");
			break;
		case 'w':
			f = fopen(filename_str, "w");
			break;
		case 'a':
			f = fopen(filename_str, "a");
			break;
		default:
			// invalid mode;
			free(filename_str);
			YASLX_print_and_throw_err_value(S, "io.open was passed invalid mode: %c.", mode_char);
		}
	}
	if (strlen(mode_str) == 2) {
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
			YASLX_print_and_throw_err_value(S, "io.open was passed invalid mode: %c+.", mode_char);
		}
	}
	if (f) {
		YASL_pushuserdata(S, f, FILE_NAME, close_file);
		YASL_loadmt(S, FILE_NAME);
		YASL_setmt(S);
	} else {
		YASL_pushundef(S);
	}
	free(filename_str);
	return 1;
}

static int YASL_io_tmpfile(struct YASL_State *S) {
	FILE *f = tmpfile();
	YASL_pushuserdata(S, f, FILE_NAME, close_file);
	YASL_loadmt(S, FILE_NAME);
	YASL_setmt(S);
	return 1;
}

static int YASL_io_setformat(struct YASL_State *S) {
	if (!YASL_isstr(S) && !YASL_isundef(S)) {
		YASLX_throw_err_type(S);
	}
	char *tmp = YASL_peekcstr(S);
	YASL_setformat(S, tmp);
	free(tmp);
	return 0;
}

static int YASL_io_read(struct YASL_State *S) {
	char mode_str[2] = { '\0', '\0' };

	if (YASL_isundef(S)) {
		mode_str[0] = 'l';
	} else if (YASL_isstr(S)) {
		char *tmp = YASL_peekcstr(S);
		if (strlen(tmp) != 1) {
			YASLX_print_err_value(S, FILE_PRE ".read was passed invalid mode: %*s.", (int)strlen(tmp), tmp);
			free(tmp);
			YASLX_throw_err_value(S);
		}
		mode_str[0] = tmp[0];
		free(tmp);
	} else {
		YASLX_print_err_bad_arg_type(S, FILE_PRE ".read", 1, YASL_STR_NAME, YASL_peekntypename(S, 1));
		YASLX_throw_err_type(S);
	}
	YASL_pop(S);

	FILE *f = YASLX_checknfile(S, FILE_PRE ".read", 0);

	switch (mode_str[0]) {
	case 'a': {
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *string = (char *) malloc(fsize);
		// Read one past the end, so hit feof (which we check below).
		size_t result = fread(string, 1, fsize + 1, f);
		string = (char *)realloc(string, result);
		if (!feof(f)) {
			YASLX_print_and_throw_err_value(S, "unable to read file");
		}
		YASL_pushlstr(S, string, result);
		free(string);
		return 1;
	}
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
		YASL_pushlstr(S, string, i);
		free(string);
		return 1;
	}
	default:
		YASLX_print_and_throw_err_value(S, FILE_PRE ".read was passed invalid mode: %c.", mode_str[0]);
	}
}

static int YASL_io_write(struct YASL_State *S) {
	FILE *f = YASLX_checknfile(S, FILE_PRE ".write", 0);

	if (!YASL_isnstr(S, 1)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, FILE_PRE ".write", 1, YASL_STR_NAME);
	}

	size_t len;
	const char *str = YASL_peeknstr(S, 1, &len);

	size_t write_len = fwrite(str, 1, len, f);

	YASL_pushint(S, write_len);
	return 1;
}

static int YASL_io_flush(struct YASL_State *S) {
	FILE *f = YASLX_checknfile(S, FILE_PRE ".flush", 0);

	int success = fflush(f);

	YASL_pushbool(S, success == 0);
	return 1;
}

static int YASL_io_seek(struct YASL_State *S) {
	FILE *f = YASLX_checknfile(S, FILE_PRE ".seek", 0);
	yasl_int offset = YASLX_checknoptint(S, FILE_PRE ".seek", 2, 0);
	YASL_pop(S);

	int whence;
	if (YASL_isundef(S)) {
		whence = SEEK_SET;
		YASL_pop(S);
	} else if (YASL_isstr(S)) {
		char *tmp = YASL_peekcstr(S);
		if (!strcmp(tmp, "set")) whence = SEEK_SET;
		else if (!strcmp(tmp, "cur")) whence = SEEK_CUR;
		else if (!strcmp(tmp, "end")) whence = SEEK_END;
		else {
			vm_print_err_value((struct VM *)S, "%s expected arg in position 1 to be one of 'set', 'cur', or 'end', got '%s'.", FILE_PRE ".seek", tmp);
			free(tmp);
			YASLX_throw_err_value(S);
		}
		free(tmp);
	} else {
		YASLX_print_err_bad_arg_type(S, FILE_PRE ".seek", 1, YASL_STR_NAME, YASL_peektypename(S));
		YASLX_throw_err_type(S);
	}

	int success = fseek(f, offset, whence);
	YASL_pushbool(S, success == 0);
	return 1;
}

static int YASL_io_close(struct YASL_State *S) {
	FILE *f = YASLX_checknfile(S, FILE_PRE ".close", 0);

	int success = fclose(f);
	YASL_pushbool(S, success == 0);
	return 1;
}

int YASL_decllib_io(struct YASL_State *S) {
	YASL_pushtable(S);
	YASL_registermt(S, FILE_NAME);

	YASL_loadmt(S, FILE_NAME);

	struct YASLX_function file_functions[] = {
		{ "read", YASL_io_read, 2 },
		{ "write", YASL_io_write, 2 },
		{ "seek", YASL_io_seek, 3 },
		{ "flush", YASL_io_flush, 1 },
		{ "close", YASL_io_close, 1 },
		{ NULL, NULL, 0 }
	};

	YASLX_tablesetfunctions(S, file_functions);
	YASL_pop(S);

	YASL_pushtable(S);
	YASLX_initglobal(S, "io");

	YASL_loadglobal(S, "io");
	YASL_pushlit(S, "open");
	YASL_pushcfunction(S, YASL_io_open, 2);
	YASL_tableset(S);

	YASL_pushlit(S, "tmpfile");
	YASL_pushcfunction(S, YASL_io_tmpfile, 0);
	YASL_tableset(S);

	YASL_pushlit(S, "setformat");
	YASL_pushcfunction(S, YASL_io_setformat, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "stdin");
	YASL_pushuserdata(S, stdin, FILE_NAME, NULL);
	YASL_loadmt(S, FILE_NAME);
	YASL_setmt(S);
	YASL_tableset(S);

	YASL_pushlit(S, "stdout");
	YASL_pushuserdata(S, stdout, FILE_NAME, NULL);
	YASL_loadmt(S, FILE_NAME);
	YASL_setmt(S);
	YASL_tableset(S);

	YASL_pushlit(S, "stderr");
	YASL_pushuserdata(S, stderr, FILE_NAME, NULL);
	YASL_loadmt(S, FILE_NAME);
	YASL_setmt(S);
	YASL_tableset(S);
	YASL_pop(S);

	return YASL_SUCCESS;
}
