#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "glob.h"

// #define LOGCHARS(a, b) printf("%c, %c\n", a, b)
#define LOGCHARS(a, b)

#define LBRACKET '['
#define RBRACKET ']'
#define RANGE '-'
#define NEGATE '!'
#define ONECHAR '?'
#define MANYCHAR '*'
#define NUM '#'
#define ESCAPE '\\'

enum {
	OK = 0,
	ERR_SYNTAX = 1
};

#define index(s, i) (LString_chars(s)[i])
#define first(s) LString_first(s)
#define next(s) LString_next(&s)
#define has_chars(s) (LString_len(s) > 0)

void glob_close_bracket(struct LString *p) {
	do {
		next(*p);
	} while (has_chars(*p) && first(*p) != RBRACKET);
}

bool glob_bracket(struct LString *pattern_ptr, struct LString *str_ptr, int *status) {
	bool b = false;
	struct LString p = *pattern_ptr;
	struct LString s = *str_ptr;

	if (!has_chars(p) || !has_chars(s)) goto end;

	do {
		if (first(p) == first(s)) {
			glob_close_bracket(&p);
			b = true;
			break;
		}
		if (index(p, 1) == RANGE && index(p, 2) != RBRACKET)  {
			const char start = index(p, 0);
			const char end = index(p, 2);
			next(p);
			next(p);
			if (start <= first(s) && first(s) <= end) {
				glob_close_bracket(&p);
				b = true;
				break;
			}
		}
		next(p);
	} while (has_chars(p) && first(p) != RBRACKET);

end:
	if (!has_chars(p)) {
		*status = ERR_SYNTAX;
		return false;
	}
	next(p);
	next(s);
	*pattern_ptr = p;
	*str_ptr = s;
	return b;
}

bool glob_internal(struct LString p, struct LString s, int *status) {
	while (has_chars(p) && has_chars(s)) {
		LOGCHARS(*pattern, *str);
		switch (first(p)) {
		case ONECHAR:
			next(p);
			next(s);
			break;
		case NUM:
			if (!isdigit(first(s))) return false;
			next(p);
			next(s);
			break;
		case LBRACKET:
			next(p);
			if (!has_chars(p)) {
				*status = ERR_SYNTAX;
				return false;
			}
			switch (first(p)) {
			case NEGATE:
				next(p);
				if (!glob_bracket(&p, &s, status)) break;
				return false;
			default:
				if (glob_bracket(&p, &s, status)) break;
				return false;
			}
			break;
		case MANYCHAR:
			next(p);
			while (has_chars(s)) {
				if (glob_internal(p, s, status)) {
					return true;
				}
				next(s);
			}
			break;
		case ESCAPE:
			next(p);
			/* fallthrough */
		default:
			if (first(p) != first(s)) return false;
			next(p);
			next(s);
			break;
		}
	}

	if (!has_chars(s)) {
		while (has_chars(p)) {
			if (first(p) != MANYCHAR) {
				return false;
			}
			next(p);
		}
		return true;
	}
	LOGCHARS(*pattern, *str);
	return !has_chars(p) && !has_chars(s);
}


bool globL(struct LString pattern, struct LString str, bool *no_error) {
	int status = OK;
	bool ret = glob_internal(pattern, str, &status);
	*no_error = status == OK;
	return ret && *no_error;
}


bool glob(const char *pattern, const char *str, bool *no_error) {
	struct LString p = {
		(char*)pattern, strlen(pattern)
	};
	struct LString s = {
		(char*)str, strlen(str)
	};

	return globL(p, s, no_error);
}

