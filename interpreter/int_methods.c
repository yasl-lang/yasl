#include "int_methods.h"

#include <stdio.h>

#include "VM.h"
#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"

static struct YASL_String *YASLX_checknstr(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnstr(S, pos)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "str", YASL_peekntypename(S, pos));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	return vm_peekstr((struct VM *)S, ((struct VM *)S)->fp + 1 + pos);
}

int int_tobool(struct YASL_State *S) {
	YASL_UNUSED(YASLX_checknint(S, "int.tobool", 0));
	YASL_pushbool(S, true);
	return 1;
}

int int_tofloat(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.tofloat", 0);
	YASL_pushfloat(S, (yasl_float) n);
	return 1;
}

int int_toint(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.toint", 0);
	YASL_pushint(S, n);
	return 1;
}

#define BUFF_LEN 67
int int_tostr(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.tostr", 0);
	const char *format = "%" PRId64 "";

	char format_char = 'd';
	if (!YASL_isundef(S)) {
		struct YASL_String *str = YASLX_checknstr(S, "int.tostr", 1);
		if (YASL_String_len(str) != 1) {
			YASL_print_err(S, MSG_VALUE_ERROR "Expected str of len 1, got str of len " PRI_SIZET ".",
				       YASL_String_len(str));
			YASL_throw_err(S, YASL_VALUE_ERROR);
		}
		format_char = *YASL_String_chars(str);
	}
	char buffer[BUFF_LEN];

	int len = 0;
	char *curr = buffer;
	switch (format_char) {
	case 'x':
		format = "0x%" PRIx64 "";
		/* fallthrough */
	case 'd':
		len = sprintf(buffer, format, n);
		break;
	case 'b':
		curr = buffer + BUFF_LEN - 1;
		do {
			len++;
			*--curr = n % 2 ? '1' : '0';
			n /= 2;
		} while (n);
		*--curr = 'b';
		*--curr = '0';
		len += 2;
		break;
	default:
		YASL_print_err(S, MSG_VALUE_ERROR "Unexpected format str: '%c'.", format_char);
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	YASL_pushlstr(S, curr, len);
	return 1;
}

int int_tochar(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.tochar", 0);
	// TODO: make this more portable.
	if (n > 127 || n < 0) {
		YASL_print_err(S, MSG_VALUE_ERROR "int.tochar was used with an invalid value (%" PRId64 ").", n);
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}
	char v = (char)n;
	YASL_pushlstr(S, &v, 1);
	return 1;
}
