#include "int_methods.h"

#include <stdio.h>

#include "VM.h"
#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"

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
		size_t len;
		const char *str = YASLX_checknstr(S, "int.tostr", 1, &len);
		if (len != 1) {
			YASLX_print_and_throw_err_value(S, "int.tostr expected str arg of len 1 in position 1, got str of len %" PRI_SIZET " ('%s').", len, str);
		}
		format_char = *str;
	}
	char buffer[BUFF_LEN];

	int len = 0;
	char *curr = buffer;
	switch (format_char) {
	case 'x':
		format = "0x%" PRIx64 "";
		/* fallthrough */
	case 'd':
	case 'r':
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
		YASLX_print_and_throw_err_value(S, "Unexpected format str for int.tostr: '%c'.", format_char);
	}

	YASL_pushlstr(S, curr, len);
	return 1;
}

int int_tochar(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.tochar", 0);
	// TODO: make this more portable.
	if (n > 255 || n < 0) {
		YASLX_print_and_throw_err_value(S, "int.tochar was used with an invalid value (%" PRId64 ").", n);
	}
	char v = (char)n;
	YASL_pushlstr(S, &v, 1);
	return 1;
}
