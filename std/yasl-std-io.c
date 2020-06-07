#include "yasl-std-io.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data-structures/YASL_Table.h"
#include "VM.h"

// what to prepend to method names in messages to user
#define FILE_PRE "io.file"

// TODO: fix mem leak in here.
static int YASL_io_open(struct YASL_State *S) {
	const char *mode_str;
	if (YASL_isundef(S)) {
		mode_str = "r";
	} else if (YASL_isstr(S)) {
		mode_str = YASL_peekcstr(S);
	} else {
		vm_print_err_bad_arg_type((struct VM *)S, "io.open", 1, Y_STR, YASL_peektype(S));
		return YASL_TYPE_ERROR;
	}
	YASL_pop(S);

	if (!YASL_isstr(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "io.open", 0, Y_STR, YASL_peektype(S));
		return YASL_TYPE_ERROR;
	}

	char *filename_str = YASL_peekcstr(S);
	YASL_pop(S);

	size_t mode_len = strlen(mode_str);

	if (mode_len > 2 || mode_len < 1 || (mode_len == 2 && mode_str[1] != '+')) {
		vm_print_err_value((struct VM *)S, "io.open was passed invalid mode: %*s.", (int)mode_len, mode_str);
		return YASL_VALUE_ERROR;
	}

	char mode_char = mode_str[0];

	FILE *f = 0;
	if (mode_len == 1) {
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
			vm_print_err_value((struct VM *)S, "io.open was passed invalid mode: %c.", mode_char);
			return YASL_VALUE_ERROR;
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
			vm_print_err_value((struct VM *)S, "io.open was passed invalid mode: %c+.", mode_char);
			return YASL_VALUE_ERROR;
		}
	}
	if (f) {
		YASL_pushuserdata(S, f, T_FILE, NULL, NULL);
		YASL_loadmt(S, FILE_PRE);
		YASL_setmt(S);
	} else {
		YASL_pushundef(S);
	}
	free(filename_str);
	return YASL_SUCCESS;
}

static int YASL_io_read(struct YASL_State *S) {
	char *mode_str;

	if (YASL_isundef(S)) {
		mode_str = (char *)malloc(2);
		mode_str[0] = 'l';
		mode_str[1] = '\0';
	} else if (YASL_isstr(S)) {
		mode_str = YASL_peekcstr(S);
	} else {
		vm_print_err_bad_arg_type((struct VM *)S, FILE_PRE ".read", 1, Y_STR, YASL_peektype(S));
		return YASL_TYPE_ERROR;
	}
	YASL_pop(S);

	if (!YASL_isuserdata(S, T_FILE)) {
		vm_print_err_type((struct VM *)S, "%s expected arg in position %d to be of type file, got arg of type %s.",
				  FILE_PRE ".read", 0, YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_popuserdata(S);

	size_t mode_len = strlen(mode_str);

	if (mode_len != 1) {
		vm_print_err_value((struct VM *)S, FILE_PRE ".read was passed invalid mode: %*s.", (int)mode_len, mode_str);
		free(mode_str);
		return YASL_VALUE_ERROR;
	}

	switch (mode_str[0]) {
	case 'a': {
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *string = (char *)malloc(fsize);
		int result = fread(string, fsize, 1, f);
		(void) result;
		// TODO clean this up.
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
		vm_print_err_value((struct VM *)S, FILE_PRE ".read was passed invalid mode: %c.", mode_str[0]);
		free(mode_str);
		return YASL_VALUE_ERROR;
	}
}

static int YASL_io_write(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, FILE_PRE ".write", 1, Y_STR, YASL_peektype(S));
		return YASL_TYPE_ERROR;
	}
	char *str = YASL_peekcstr(S);
	YASL_pop(S);

	if (!YASL_isuserdata(S, T_FILE)) {
		vm_print_err_type((struct VM *)S, "%s expected arg in position %d to be of type file, got arg of type %s.",
				  FILE_PRE ".write", 0, YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_popuserdata(S);

	size_t len = strlen(str);

	size_t write_len = fwrite(str, 1, len, f);

	YASL_pushint(S, write_len);
	free(str);
	return YASL_SUCCESS;
}

static int YASL_io_flush(struct YASL_State *S) {
	if (!YASL_isuserdata(S, T_FILE)) {
		vm_print_err_type((struct VM *)S, "%s expected arg in position %d to be of type file, got arg of type %s.",
				  FILE_PRE ".flush", 0, YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_popuserdata(S);

	int success = fflush(f);

	YASL_pushbool(S, success == 0);
	return YASL_SUCCESS;
}

static int YASL_io_seek(struct YASL_State *S) {
	yasl_int offset;
	if (YASL_isundef(S)) {
		offset = 0;
		YASL_pop(S);
	} else if ((YASL_isint(S))) {
		offset = YASL_popint(S);
	} else {
		vm_print_err_type((struct VM *)S,"%s expected arg in position %d to be of type int, got arg of type %s.", FILE_PRE ".seek", 2, YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}

	int whence;
	if (YASL_isundef(S)) {
		whence = SEEK_SET;
		YASL_pop(S);
	} else if (YASL_isstr(S)) {
		char *tmp = YASL_popcstr(S);
		if (!strcmp(tmp, "set")) whence = SEEK_SET;
		else if (!strcmp(tmp, "cur")) whence = SEEK_CUR;
		else if (!strcmp(tmp, "end")) whence = SEEK_END;
		else {
			vm_print_err_value((struct VM *)S, "%s expected arg in position 1 to be one of 'set', 'cur', or 'end', got '%s'.", FILE_PRE ".seek", tmp);
			free(tmp);
			return YASL_VALUE_ERROR;
		}
		free(tmp);
	} else {
		vm_print_err_bad_arg_type((struct VM *)S, FILE_PRE ".seek", 1, Y_STR, YASL_peektype(S));
		return YASL_TYPE_ERROR;
	}

	if (!YASL_isuserdata(S, T_FILE)) {
		vm_print_err_type((struct VM *)S, "%s expected arg in position %d to be of type file, got arg of type %s.",
				  FILE_PRE ".seek", 0, YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_popuserdata(S);

	int success = fseek(f, offset, whence);
	YASL_pushbool(S, success == 0);
	return YASL_SUCCESS;
}

static int YASL_io_close(struct YASL_State *S) {
	if (!YASL_isuserdata(S, T_FILE)) {
		vm_print_err_type((struct VM *)S, "%s expected arg in position %d to be of type file, got arg of type %s.",
				  FILE_PRE ".flush", 0, YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	FILE *f = (FILE *)YASL_popuserdata(S);

	int success = fclose(f);
	YASL_pushbool(S, success == 0);
	return YASL_SUCCESS;
}

int YASL_decllib_io(struct YASL_State *S) {
	YASL_pushtable(S);
	YASL_registermt(S, FILE_PRE);

	YASL_loadmt(S, FILE_PRE);
	YASL_pushlitszstring(S, "read");
	YASL_pushcfunction(S, YASL_io_read, 2);
	YASL_tableset(S);

	YASL_loadmt(S, FILE_PRE);
	YASL_pushlitszstring(S, "write");
	YASL_pushcfunction(S, YASL_io_write, 2);
	YASL_tableset(S);

	YASL_loadmt(S, FILE_PRE);
	YASL_pushlitszstring(S, "seek");
	YASL_pushcfunction(S, YASL_io_seek, 3);
	YASL_tableset(S);

	YASL_loadmt(S, FILE_PRE);
	YASL_pushlitszstring(S, "flush");
	YASL_pushcfunction(S, YASL_io_flush, 1);
	YASL_tableset(S);

	YASL_loadmt(S, FILE_PRE);
	YASL_pushlitszstring(S, "close");
	YASL_pushcfunction(S, YASL_io_close, 1);
	YASL_tableset(S);


	YASL_declglobal(S, "io");
	YASL_pushtable(S);
	YASL_setglobal(S, "io");

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "open");
	YASL_pushcfunction(S, YASL_io_open, 2);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "read");
	YASL_pushcfunction(S, YASL_io_read, 2);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "write");
	YASL_pushcfunction(S, YASL_io_write, 2);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "seek");
	YASL_pushcfunction(S, YASL_io_seek, 3);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "flush");
	YASL_pushcfunction(S, YASL_io_flush, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "close");
	YASL_pushcfunction(S, YASL_io_close, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "stdin");
	YASL_pushuserdata(S, stdin, T_FILE, NULL, NULL);
	YASL_loadmt(S, FILE_PRE);
	YASL_setmt(S);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "stdout");
	YASL_pushuserdata(S, stdout, T_FILE, NULL, NULL);
	YASL_loadmt(S, FILE_PRE);
	YASL_setmt(S);
	YASL_tableset(S);

	YASL_loadglobal(S, "io");
	YASL_pushlitszstring(S, "stderr");
	YASL_pushuserdata(S, stderr, T_FILE, NULL, NULL);
	YASL_loadmt(S, FILE_PRE);
	YASL_setmt(S);
	YASL_tableset(S);

	return YASL_SUCCESS;
}
