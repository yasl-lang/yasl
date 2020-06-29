#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "debug.h"
#include "yasl_error.h"
#include "yasl_include.h"

static enum Token YASLToken_ThreeChars(char c1, char c2, char c3);
static enum Token YASLToken_TwoChars(char c1, char c2);
static enum Token YASLToken_OneChar(char c1);
static void YASLKeywords(struct Lexer *const lex);

static int isbdigit(int c) {
	return c == '0' || c == '1';
}

static int isddigit(int c) {
	return isdigit(c);
}

static bool iswhitespace(int c) {
	return c == ' ' || c == '\n' || c == '\t';
}

YASL_FORMAT_CHECK static void lex_print_err(struct Lexer *lex, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	lex->err.print(&lex->err, fmt, args);
	va_end(args);
}

#define lex_print_err_syntax(lex, format, ...) lex_print_err(lex, "SyntaxError: " format, __VA_ARGS__)

#define isyaslidstart(c) (isalpha(c) || (c) == '_' || (c) == '$')
#define isyaslid(c) (isalnum(c) || (c) == '_' || (c) == '$')

void lex_error(struct Lexer *const lex) {
	free(lex->value);
	lex->value = NULL;
	lex->type = T_UNKNOWN;
	lex->status = YASL_SYNTAX_ERROR;
}

int lex_getchar(struct Lexer *const lex) {
	return lex->c = (char)lxgetc(lex->file);
}

void lex_val_init(struct Lexer *const lex) {
	lex->val_cap = 8;
	lex->val_len = 0;
	lex->value = (char *)realloc(lex->value, lex->val_cap);
}

void lex_val_append(struct Lexer *const lex, char c) {
	if (lex->val_len == lex->val_cap) {
		lex->val_cap *= 2;
		lex->value = (char *)realloc(lex->value, lex->val_cap);
	}
	lex->value[lex->val_len++] = c;
}

static bool lex_eatwhitespace(struct Lexer *const lex) {
	while (!lxeof(lex->file) && iswhitespace(lex->c)) {
		if (lex->c == '\n') {
			lex->line++;
			if (ispotentialend(lex)) {
				lex->type = T_SEMI;
				return true;
			}
		}
		lex_getchar(lex);
	}
	return false;
}

static bool lex_eatinlinecomments(struct Lexer *const lex) {
	if ('#' == lex->c) while (!lxeof(lex->file) && lex_getchar(lex) != '\n') ;
	return false;
}

static bool lex_eatcommentsandwhitespace(struct Lexer * lex) {
	while (!lxeof(lex->file) && (iswhitespace(lex->c) || lex->c == '#' || lex->c == '/')) {
		// white space
		if (lex_eatwhitespace(lex)) return true;

		// inline comments
		if (lex_eatinlinecomments(lex)) return true;

		// block comments
		if (lex->c == '/') {
		    long cur = lxtell(lex->file);
			lex_getchar(lex);
			if (lex->c == '*') {
				int addsemi = 0;
				lex->c = ' ';
				int c1 = lxgetc(lex->file);
				int c2 = lxgetc(lex->file);
				while (!lxeof(lex->file) && (c1 != '*' || c2 != '/')) {
					if (c1 == '\n' || c2 == '\n') addsemi = 1;
					if (c1 == '\n') lex->line++;
					c1 = c2;
					c2 = lxgetc(lex->file);
				}
				if (lxeof(lex->file)) {
					lex_print_err_syntax(lex,  "Unclosed block comment in line %" PRI_SIZET ".\n", lex->line);
					lex_error(lex);
					return true;
				}
				if (addsemi && ispotentialend(lex)) {
					lex->type = T_SEMI;
					return true;
				}
			} else if (lxeof(lex->file)) {
				lex->type = T_SLASH;
				return true;
			} else {
			    lxseek(lex->file, cur, SEEK_SET);
			    lex->c = '/';
				break;
			}
		}
	}
	return false;
}

static bool lex_eatop(struct Lexer *const lex) {
	char c1, c2, c3;
	enum Token last;
	c1 = lex->c;
	long cur_one = lxtell(lex->file);
	long cur_two = cur_one;
	c2 = lxgetc(lex->file);
	if (lxeof(lex->file)) {
		goto one;
	}

	cur_two = lxtell(lex->file);
	c3 = lxgetc(lex->file);
	if (lxeof(lex->file)) {
		goto two;
	}

	last = YASLToken_ThreeChars(c1, c2, c3);
	if (last != T_UNKNOWN) {
		lex->type = last;
		free(lex->value);
		return true;
	}
	lxseek(lex->file, cur_two, SEEK_SET);

	two:
	last = YASLToken_TwoChars(c1, c2);
	if (last != T_UNKNOWN) {
		lex->type = last;
		free(lex->value);
		return true;
	}
	lxseek(lex->file, cur_one, SEEK_SET);

	one:
	last = YASLToken_OneChar(c1);
	if (last != T_UNKNOWN) {
		lex->type = last;
		free(lex->value);
		return true;
	}
	return false;
}

static long lex_eatnumber_fill(struct Lexer *const lex, int (*isvaliddigit)(int)) {
	long cur;
	do {
		lex_val_append(lex, lex->c);
		cur = lxtell(lex->file);
		lex_getchar(lex);
		while (lex->c == NUM_SEPERATOR) {
			cur = lxtell(lex->file);
			lex_getchar(lex);
		}
	} while (!lxeof(lex->file) && (*isvaliddigit)(lex->c));
	return cur;
}

static bool lex_eatint(struct Lexer *const lex, char separator, int (*isvaliddigit)(int)) {
	int curr_pos = lxtell(lex->file);
	int curr_char = lex->c;
	if (lex->c == '0') {
		lex_getchar(lex);
		lex_val_append(lex, '0');
		if (tolower(lex->c) == separator) {
			lex_getchar(lex);
			lex_val_append(lex, separator);
			while (lex->c == NUM_SEPERATOR) lex_getchar(lex);

			if (!(*isvaliddigit)(lex->c)) {
				lex_print_err_syntax(lex, "Invalid int literal in line %" PRI_SIZET ".\n", lex->line);
				lex_error(lex);
				return true;
			}

			long cur = lex_eatnumber_fill(lex, isvaliddigit);
			lex_val_append(lex, '\0');
			lex->type = T_INT;
			if (!lxeof(lex->file)) lxseek(lex->file, cur, SEEK_SET);
			return true;
		}
	}
	lex->val_len = 0;
	lex->c = curr_char;
	lxseek(lex->file, curr_pos, SEEK_SET);
	return false;
}

static bool lex_eatfloat(struct Lexer *const lex) {
	if (lex->c == '.') {
		long cur = lxtell(lex->file);
		lex_getchar(lex);
		if (lxeof(lex->file)) {
			lxseek(lex->file, cur - 1, SEEK_SET);
			lex->c = '.';
			return true;
		}
		if (!isdigit(lex->c)) {
			lxseek(lex->file, cur - 1, SEEK_SET);
			return true;
		}
		lex->val_len--;
		lex_val_append(lex, '.');
		lex_eatnumber_fill(lex, &isddigit);

		if (tolower(lex->c) == 'e') {
			lex_getchar(lex);
			lex_val_append(lex, 'e');
			while (lex->c == NUM_SEPERATOR) lex_getchar(lex);
			lex_eatnumber_fill(lex, &isddigit);
		}

		lex_val_append(lex, '\0');
		if (!lxeof(lex->file)) lxseek(lex->file, -1, SEEK_CUR);
		lex->type = T_FLOAT;
		return true;
	}
	return false;
}

static bool lex_eatnumber(struct Lexer *const lex) {
	if (isdigit(lex->c)) {                          // numbers
		lex_val_init(lex);

		// hexadecimal literal
		if (lex_eatint(lex, 'x', &isxdigit)) return true;

		// binary literal
		if (lex_eatint(lex, 'b', &isbdigit)) return true;

		// decimal (or first half of float)
		long cur = lex_eatnumber_fill(lex, &isddigit);

		lex->type = T_INT;
		lex_val_append(lex, '\0');

		// floats
		if (lex_eatfloat(lex)) return true;

		if (!lxeof(lex->file)) lxseek(lex->file, cur, SEEK_SET);
		return true;
	}
	return false;
}

static void lex_eatid_fill(struct Lexer *const lex) {
    long cur;
	do {
		lex_val_append(lex, lex->c);
		cur = lxtell(lex->file);
		lex_getchar(lex);
	} while (!lxeof(lex->file) && isyaslid(lex->c));
    if (!lxeof(lex->file)) lxseek(lex->file, cur, SEEK_SET);
}
static bool lex_eatid(struct Lexer *const lex) {
	if (isyaslidstart(lex->c)) {                           // identifiers and keywords
		lex_val_init(lex);
		lex_eatid_fill(lex);

		lex_val_append(lex, '\0');

		switch (lex->type) {
		case T_DOT:
		case T_RIGHT_ARR:
			lex->type = T_ID;
			break;
		default:
			lex->type = T_ID;
			YASLKeywords(lex);                     // keywords
			break;
		}

		return true;
	}
	return false;
}

static bool handle_escapes(struct Lexer *const lex, char delim) {
	char buffer[9];
	char tmp;
	char *end;
	switch (lex->c) {
	case 'a':
		lex_val_append(lex, '\a');
		break;
	case 'b':
		lex_val_append(lex, '\b');
		break;
	case 'f':
		lex_val_append(lex, '\f');
		break;
	case 'n':
		lex_val_append(lex, '\n');
		break;
	case 'r':
		lex_val_append(lex, '\r');
		break;
	case 't':
		lex_val_append(lex, '\t');
		break;
	case 'v':
		lex_val_append(lex, '\v');
		break;
	case '0':
		lex_val_append(lex, '\0');
		break;
	case STR_DELIM:
	case INTERP_STR_DELIM:
	case INTERP_STR_PLACEHOLDER:
	case ESCAPE_CHAR:
		lex_val_append(lex, lex->c);
		break;
	case 'x':
		buffer[0] = lex_getchar(lex);
		buffer[1] = lex_getchar(lex);
		buffer[2] = '\0';
		tmp = (char) strtol(buffer, &end, 16);
		if (end != buffer + 2) {
			lex_print_err_syntax(lex, "Invalid hex string escape in line %" PRI_SIZET ".\n", lex->line);
			goto error;
		}
		lex_val_append(lex, tmp);
		return false;
	default:
		lex_print_err_syntax(lex, "Invalid string escape sequence in line %" PRI_SIZET ".\n", lex->line);
		goto error;
	}
	return false;

error:
	while (lex->c != '\n' && lex->c != delim && !lxeof(lex->file)) lex_getchar(lex);
	lex_error(lex);
	return true;
}

static bool lex_eatstring_nextchar(struct Lexer *const lex, char delim) {
	if (lex->c == ESCAPE_CHAR) {
		lex_getchar(lex);
		if (handle_escapes(lex, delim)) {
			return true;
		}
	} else {
		lex_val_append(lex, lex->c);
	}
	return false;
}

static bool lex_eatinterpstring_fill(struct Lexer *const lex) {
	while (lex->c != INTERP_STR_DELIM && lex->c != INTERP_STR_PLACEHOLDER && !lxeof(lex->file)) {
		if (lex->c == '\n') {
			lex_print_err_syntax(lex,  "Unclosed string literal in line %" PRI_SIZET ".\n", lex->line);
			lex_error(lex);
			return true;
		}

		if (lex_eatstring_nextchar(lex, INTERP_STR_DELIM)) return true;

		lex_getchar(lex);
	}

	if (lex->c == INTERP_STR_PLACEHOLDER) {
		lex->mode = L_INTERP;
	} else {
		lex->mode = L_NORMAL;
	}

	return false;
}

void lex_eatinterpstringbody(struct Lexer *const lex) {
	lex_val_init(lex);
	lex->type = T_STR;

	if (lex_eatinterpstring_fill(lex)) return;

	if (lxeof(lex->file)) {
		lex_print_err_syntax(lex,  "Unclosed string literal in line %" PRI_SIZET ".\n", lex->line);
		lex_error(lex);
		return;
	}
}

static bool lex_eatinterpstring(struct Lexer *const lex) {
	if (lex->c == INTERP_STR_DELIM) {
		lex_getchar(lex);
		lex_eatinterpstringbody(lex);
		return true;
	}
	return false;
}

static bool lex_eatstring(struct Lexer *const lex) {
	if (lex->c == STR_DELIM) {
		lex_val_init(lex);
		lex->type = T_STR;

		lex_getchar(lex);
		while (lex->c != STR_DELIM && !lxeof(lex->file)) {
			if (lex->c == '\n') {
				lex_print_err_syntax(lex, "Unclosed string literal in line %" PRI_SIZET ".\n", lex->line);
				lex_error(lex);
				return true;
			}

			if (lex_eatstring_nextchar(lex, STR_DELIM)) return true;

			lex_getchar(lex);
		}

		if (lxeof(lex->file)) {
			lex_print_err_syntax(lex, "Unclosed string literal in line %" PRI_SIZET ".\n", lex->line);
			lex_error(lex);
			return true;
		}

		return true;

	}
	return false;
}

static bool lex_eatrawstring(struct Lexer *const lex) {
	if (lex->c == RAW_STR_DELIM) {
		lex_val_init(lex);
		lex->type = T_STR;

		lex_getchar(lex);
		while (lex->c != RAW_STR_DELIM && !lxeof(lex->file)) {
			if (lex->c == '\n') lex->line++;
			lex_val_append(lex, lex->c);
			lex_getchar(lex);
		}

		if (lxeof(lex->file)) {
			lex_print_err_syntax(lex, "Unclosed string literal in line %" PRI_SIZET ".\n", lex->line);
			lex_error(lex);
			return true;
		}

		return true;

	}
	return false;
}


void gettok(struct Lexer *const lex) {
	YASL_LEX_DEBUG_LOG("getting token from line %" PRI_SIZET "\n", lex->line);
	lex->value = NULL;
	lex_getchar(lex);

	// whitespace and comments.
	if (lex_eatcommentsandwhitespace(lex)) return;

	// EOF
	if (lxeof(lex->file)) {
		lex->type = T_EOF;
		lex->value = NULL;
		return;
	}

	// numbers
	if (lex_eatnumber(lex)) return;

	// identifiers and keywords
	if (lex_eatid(lex)) return;

	// strings
	if (lex_eatstring(lex)) return;

	// raw strings
	if (lex_eatrawstring(lex)) return;

	// interpolated strings
	if (lex_eatinterpstring(lex)) return;

	// operators
	if (lex_eatop(lex)) return;

	lex_print_err_syntax(lex, "Unknown character in line %" PRI_SIZET ": `%c` (0x%x).\n", lex->line, lex->c, lex->c);
	lex_error(lex);
}

static enum Token YASLToken_ThreeChars(char c1, char c2, char c3) {
	switch(c1) {
	case '.': switch(c2) { case '.': switch(c3) { case '.': return T_TDOT;} } return T_UNKNOWN;
	case '<': switch(c2) { case '<': switch(c3) { case '=': return T_DLTEQ;} } return T_UNKNOWN;
	case '>': switch(c2) { case '>': switch(c3) { case '=': return T_DGTEQ; } } return T_UNKNOWN;
	case '=': switch(c2) { case '=': switch(c3) { case '=': return T_TEQ; } } return T_UNKNOWN;
		case '!': switch(c2) {
			case '=': switch(c3) { case '=': return T_BANGDEQ;}
			  return T_UNKNOWN;
	  } return T_UNKNOWN;
	case '*': switch(c2) { case '*': switch(c3) { case '=': return T_DSTAREQ; }} return T_UNKNOWN;
	case '/': switch(c2) { case '/': switch(c3) { case '=': return T_DSLASHEQ; }} return T_UNKNOWN;
		case '&': switch(c2) {
	  case '&': switch(c3) { case '=': return T_DAMPEQ; } return T_UNKNOWN;
			case '^': switch(c3) { case '=': return T_AMPCARETEQ;}
			  return T_UNKNOWN;
	  } return T_UNKNOWN;
		case '|': switch(c2) { case '|': switch(c3) {
			case '=': return T_DBAREQ;
	    } } return T_UNKNOWN;
		case '?': switch(c2) { case '?': switch(c3) { case '=': return T_DQMARKEQ; } }
	}
	return T_UNKNOWN;
}

static enum Token YASLToken_TwoChars(char c1, char c2) {
	switch(c1) {
	case ':': switch(c2) { case '=': return T_COLONEQ; } return T_UNKNOWN;
	case '^': switch(c2) { case '=': return T_CARETEQ; } return T_UNKNOWN;
	case '+': switch(c2) { case '=': return T_PLUSEQ; } return T_UNKNOWN;
		case '-': switch(c2) {
				case '=': return T_MINUSEQ;
				case '>': return T_RIGHT_ARR;
	  } return T_UNKNOWN;
		case '=': switch(c2) {
			case '=': return T_DEQ;
			case '~': return T_EQTILDE;
	  } return T_UNKNOWN;
		case '!': switch(c2) {
			case '=': return T_BANGEQ;
			case '~': return T_BANGTILDE;
	  } return T_UNKNOWN;
	case '~': switch(c2) { case '=': return T_TILDEEQ; } return T_UNKNOWN;
		case '*': switch(c2) {
			case '=': return T_STAREQ;
			case '*': return T_DSTAR;
	  } return T_UNKNOWN;
		case '/': switch(c2) {
				case '=': return T_SLASHEQ;
	  case '/': return T_DSLASH;} return T_UNKNOWN;
	case '%': switch(c2) { case '=': return T_MODEQ; } return T_UNKNOWN;
		case '<': switch(c2) {
				case '=': return T_LTEQ;
				case '<': return T_DLT;
				case '-': return T_LEFT_ARR;
	  } return T_UNKNOWN;
		case '>': switch(c2) {
				case '=': return T_GTEQ;
				case '>': return T_DGT;
	  } return T_UNKNOWN;
		case '&': switch(c2) {
			case '=': return T_AMPEQ;
			case '&': return T_DAMP;
			case '^': return T_AMPCARET;
	  } return T_UNKNOWN;
		case '|': switch(c2) {
				case '=': return T_BAREQ;
				case '|': return T_DBAR;
	  } return T_UNKNOWN;
	case '?': switch(c2) { case '?': return T_DQMARK; } return T_UNKNOWN;
		default:
			return T_UNKNOWN;
	}
}

static enum Token YASLToken_OneChar(char c1) {
	switch (c1) {
	case ';': return T_SEMI;
	case '(': return T_LPAR;
	case ')': return T_RPAR;
	case '[': return T_LSQB;
	case ']': return T_RSQB;
	case '{': return T_LBRC;
	case '}': return T_RBRC;
	case '.': return T_DOT;
	case ',': return T_COMMA;
	case '^': return T_CARET;
	case '+': return T_PLUS;
	case '-': return T_MINUS;
	case '!': return T_BANG;
	case '~': return T_TILDE;
	case '*': return T_STAR;
	case '/': return T_SLASH;
	case '%': return T_MOD;
	case '<': return T_LT;
	case '>': return T_GT;
	case '=': return T_EQ;
	case '&': return T_AMP;
	case '|': return T_BAR;
	case '?': return T_QMARK;
	case ':': return T_COLON;
	default: return T_UNKNOWN;
	}
}

static bool matches_keyword(struct Lexer *const lex, const char *string) {
	return !strcmp(lex->value, string);
}

static void set_keyword(struct Lexer *const lex, enum Token type) {
	lex->type = type;
	free(lex->value);
	lex->value = NULL;
}

static void YASLKeywords(struct Lexer *const lex) {
	/* keywords:
	 *  let
	 *  print
	 *  if
	 *  in
	 *  elseif
	 *  else
	 *  while
	 *  for
	 *  break
	 *  continue
	 *  true
	 *  false
	 *  or
	 *  and
	 *  undef
	 *  fn
	 *  return
	 *  assert
	 */

	if (matches_keyword(lex, "header")) {
		lex_print_err_syntax(lex,  "`header` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "global")) {
		lex_print_err_syntax(lex,  "`global` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "enum")) {
		lex_print_err_syntax(lex,  "`enum` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "struct")) {
		lex_print_err_syntax(lex,  "`struct` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "pragma")) {
		lex_print_err_syntax(lex,  "`pragma` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "yield")) {
		lex_print_err_syntax(lex,  "`yield` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "do")) {
		lex_print_err_syntax(lex,  "`do` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "ifdef")) {
		lex_print_err_syntax(lex,  "`ifdef` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "elseifdef")) {
		lex_print_err_syntax(lex,  "`elseifdef` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "use")) {
		lex_print_err_syntax(lex,  "`use` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "no")) {
		lex_print_err_syntax(lex,  "`no` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "pure")) {
		lex_print_err_syntax(lex,  "`pure` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "consteval")) {
		lex_print_err_syntax(lex,  "`consteval` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n",
					lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "constexpr")) {
		lex_print_err_syntax(lex,  "`constexpr` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n",
					lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "constfold")) {
		lex_print_err_syntax(lex,  "`constfold` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n",
					lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "extern")) {
		lex_print_err_syntax(lex,  "`extern` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n",
					lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "in")) {
		lex_print_err_syntax(lex,  "`in` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n",
				     lex->line);
		lex_error(lex);
		return;
	}

	if (matches_keyword(lex, "break")) set_keyword(lex, T_BREAK);
	else if (matches_keyword(lex, "const")) set_keyword(lex, T_CONST);
	else if (matches_keyword(lex, "continue")) set_keyword(lex, T_CONT);
	else if (matches_keyword(lex, "else")) set_keyword(lex, T_ELSE);
	else if (matches_keyword(lex, "elseif")) set_keyword(lex, T_ELSEIF);
	else if (matches_keyword(lex, "export")) set_keyword(lex, T_EXPORT);
	else if (matches_keyword(lex, "fn")) set_keyword(lex, T_FN);
	else if (matches_keyword(lex, "for")) set_keyword(lex, T_FOR);
	else if (matches_keyword(lex, "if")) set_keyword(lex, T_IF);
	else if (matches_keyword(lex, "echo")) set_keyword(lex, T_ECHO);
	else if (matches_keyword(lex, "let")) set_keyword(lex, T_LET);
	else if (matches_keyword(lex, "return")) set_keyword(lex, T_RET);
	else if (matches_keyword(lex, "undef")) set_keyword(lex, T_UNDEF);
	else if (matches_keyword(lex, "while")) set_keyword(lex, T_WHILE);
	else if (matches_keyword(lex, "len")) set_keyword(lex, T_LEN);
	else if (matches_keyword(lex, "assert")) set_keyword(lex, T_ASS);
	else if (matches_keyword(lex, "match")) set_keyword(lex, T_MATCH);
	// NOTE: special case for bools
	else if (matches_keyword(lex, "true") || matches_keyword(lex, "false")) lex->type = T_BOOL;
}

// Note: keep in sync with token.h
const char *YASL_TOKEN_NAMES[] = {
	"END OF FILE",  // T_EOF,
	";",            // T_SEMI,
	"undef",        // T_UNDEF,
	"float",        // T_FLOAT,
	"int",          // T_INT,
	"bool",         // T_BOOL,
	"str",          // T_STR,
	"if",           // T_IF,
	"elseif",       // T_ELSEIF,
	"else",         // T_ELSE,
	"while",        // T_WHILE,
	"break",        // T_BREAK,
	"continue",     // T_CONT,
	"for",          // T_FOR,
	"id",           // T_ID,
	"const",        // T_CONST,
	"fn",           // T_FN,
	"let",          // T_LET,
	"return",       // T_RET,
	"export",       // T_EXPORT,
	"echo",         // T_PRINT,
	"len",          // T_LEN,
	"assert",       // T_ASS,
	"match",        // T_MATCH,
	"(",            // LPAR,
	")",            // RPAR,
	"[",            // LSQB,
	"]",            // RSQB,
	"{",            // LBRC,
	"}",            // RBRC,
	".",            // DOT,
	"...",          // T_DOT
	",",            // COMMA,
	"^",            // CARET,
	"^=",           //CARETEQ,
	"+",            // PLUS,
	"+=",           // PLUSEQ,
	"-",            // MINUS,
	"-=",           // MINUSEQ,
	"!",            // BANG,
	"!=",           // BANGEQ,
	"!==",          // BANGDEQ,
	"!~",           // BANGTILDE,
	"~",            // TILDE,
	"~=",           // TILDEEQ,
	"*",            // STAR,
	"*=",           // STAREQ,
	"**",           // DSTAR,
	"**=",          // DSTAREQ,
	"/",            // SLASH,
	"/=",           // SLASHEQ,
	"//",           // DSLASH,
	"//=",          // DSLASHEQ,
	"%",            // MOD,
	"%=",           // MODEQ,
	"<",            // LT,
	"<=",           // LTEQ,
	"<<",           // DLT,
	"<<=",          // DLTEQ,
	">",            // GT,
	">=",           // GTEQ,
	">>",           // DGT,
	">>=",          // DGTEQ,
	"=",            // EQ,
	"==",           // DEQ,
	"===",          // TEQ,
	"=~",           // EQTILDE,
	"&",            // AMP,
	"&=",           // AMPEQ,
	"&&",           // DAMP,
	"&&=",          // DAMPEQ,
	"&^",           // AMPCARET,
	"&^=",          // AMPCARETEQ,
	"|",            // BAR,
	"|=",           // BAREQ,
	"||",           // DBAR,
	"||=",          // DBAREQ,
	"?",            // QMARK,
	"??",           // DQMARK,
	"?\?=",         // DQMARKEQ,
	":",            // COLON,
	":=",           // COLONEQ
	"->",           // RIGHT_ARR
	"<-",           // LEFT_ARR
};

struct Lexer *lex_new(FILE *file /* OWN */) {
	struct Lexer *const lex = (struct Lexer *) malloc(sizeof(struct Lexer));
	lex->line = 1;
	lex->value = NULL;
	lex->val_cap = 0;
	lex->val_len = 0;
	lex->file = lexinput_new_file(file);
	lex->type = T_UNKNOWN;
	lex->status = YASL_SUCCESS;
	lex->mode = L_NORMAL;
	return lex;
}

void lex_cleanup(struct Lexer *const lex) {
	lxclose(lex->file);
}
