#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common/debug.h"
#include "yasl_error.h"
#include "yasl_include.h"

static enum Token YASLToken_ThreeChars(char c1, char c2, char c3);
static enum Token YASLToken_TwoChars(char c1, char c2);
static enum Token YASLToken_OneChar(char c1);

static int isbdigit(int c) {
	return c == '0' || c == '1';
}

static int isddigit(int c) {
	return isdigit(c);
}

static bool iswhitespace(int c) {
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

YASL_FORMAT_CHECK static void lex_print_err(struct Lexer *lex, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	lex->err.print(&lex->err, fmt, args);
	va_end(args);
}

#define lex_print_err_syntax(lex, format, ...) lex_print_err(lex, "SyntaxError: " format, __VA_ARGS__)

void lex_val_setnull(struct Lexer *const lex) {
	lex->buffer.items = NULL;
	lex->buffer.size = 0;
	lex->buffer.count = 0;
}

static void lex_val_init(struct Lexer *const lex) {
	lex->buffer.size = 8;
	lex->buffer.count = 0;
	lex->buffer.items = (unsigned char *)realloc(lex->buffer.items, lex->buffer.size);
}

void lex_val_free(struct Lexer *const lex) {
	free(lex->buffer.items);
}

static void lex_val_append(struct Lexer *const lex, char c) {
	YASL_ByteBuffer_add_byte(&lex->buffer, (unsigned char)c);
}

char *lex_val_get(struct Lexer *const lex) {
	return (char *)lex->buffer.items;
}

void lex_val_save(const struct Lexer *const lex, YASL_ByteBuffer *const buffer) {
	*buffer = lex->buffer;
}

void lex_val_restore(struct Lexer *const lex, const YASL_ByteBuffer *const buffer) {
	lex->buffer = *buffer;
}

void lex_error(struct Lexer *const lex) {
	lex_val_free(lex);
	lex_val_setnull(lex);
	lex->type = T_UNKNOWN;
	lex->status = YASL_SYNTAX_ERROR;
}

int lex_getchar(struct Lexer *const lex) {
	return lex->c = (char)lxgetc(lex->file);
}

bool lex_eatwhitespace(struct Lexer *const lex) {
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

#define COMMENT_START '/'
#define COMMENT_END '/'
static bool lex_eatcommentsandwhitespace(struct Lexer * lex) {
	while (!lxeof(lex->file) && (iswhitespace(lex->c) || lex->c == '#' || lex->c == COMMENT_START)) {
		// white space
		if (lex_eatwhitespace(lex)) return true;

		// inline comments
		if (lex_eatinlinecomments(lex)) return true;

		// block comments
		if (lex->c == COMMENT_START) {
		    long cur = lxtell(lex->file);
			lex_getchar(lex);
			if (lex->c == '*') {
				bool addsemi = false;
				lex->c = ' ';
				int c1 = lxgetc(lex->file);
				int c2 = lxgetc(lex->file);
				while (!lxeof(lex->file) && (c1 != '*' || c2 != COMMENT_END)) {
					if (c1 == '\n' || c2 == '\n') addsemi = true;
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
			} else {
			    lxseek(lex->file, cur, SEEK_SET);
			    lex->c = COMMENT_START;
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
		lex_val_free(lex);
		return true;
	}
	lxseek(lex->file, cur_two, SEEK_SET);

	two:
	last = YASLToken_TwoChars(c1, c2);
	if (last != T_UNKNOWN) {
		lex->type = last;
		lex_val_free(lex);
		return true;
	}
	lxseek(lex->file, cur_one, SEEK_SET);

	one:
	last = YASLToken_OneChar(c1);
	switch (last) {
	case T_LPAR:
		lex->par_count++;
		break;
	case T_RPAR:
		lex->par_count--;
		break;
	case T_LSQB:
		lex->sqb_count++;
		break;
	case T_RSQB:
		lex->sqb_count--;
		break;
	case T_LBRC:
		lex->brc_count++;
		break;
	case T_RBRC:
		lex->brc_count--;
		break;
	default:
		break;
	}
	if (last != T_UNKNOWN) {
		lex->type = last;
		lex_val_free(lex);
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
	lex->buffer.count = 0;
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
		lex->buffer.count--;
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
#define X(escape, c) case c: lex_val_append(lex, escape); break;
#include "escapes.x"
#undef X
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
	lex_val_setnull(lex);
	lex_getchar(lex);

	// whitespace and comments.
	if (lex_eatcommentsandwhitespace(lex)) return;

	// EOF
	if (lxeof(lex->file)) {
		if (ispotentialend(lex)) {
			lex->type = T_SEMI;
			return;
		}
		lex->type = T_EOF;
		lex_val_setnull(lex);
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

	lex_print_err_syntax(lex, "Unknown character in line %" PRI_SIZET ": 0x%x (`%c`).\n", lex->line, lex->c, lex->c);
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
	case '^': switch(c2) { case '=': return T_CARETEQ; } return T_UNKNOWN;
	case '+': switch(c2) { case '=': return T_PLUSEQ; } return T_UNKNOWN;
		case '-': switch(c2) {
				case '=': return T_MINUSEQ;
				case '>': return T_RIGHT_ARR;
	  } return T_UNKNOWN;
		case '=': switch(c2) {
			case '=': return T_DEQ;
	  } return T_UNKNOWN;
		case '!': switch(c2) {
			case '=': return T_BANGEQ;
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
	return !strcmp(lex_val_get(lex), string);
}

static void set_keyword(struct Lexer *const lex, enum Token type) {
	lex->type = type;
	lex_val_free(lex);
	lex_val_setnull(lex);
}

void YASLKeywords(struct Lexer *const lex) {
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

#define X(word, ...) if (matches_keyword(lex, #word)) {\
	lex_print_err_syntax(lex, "`" #word "` is an unused reserved word and cannot be used (line %" PRI_SIZET ").\n", lex->line);\
	lex_error(lex);\
	return;\
}

#include "reservedwords.x"
#undef X

	if (matches_keyword(lex, "break")) set_keyword(lex, T_BREAK);
	else if (matches_keyword(lex, "const")) set_keyword(lex, T_CONST);
	else if (matches_keyword(lex, "continue")) set_keyword(lex, T_CONT);
	else if (matches_keyword(lex, "else")) set_keyword(lex, T_ELSE);
	else if (matches_keyword(lex, "elseif")) set_keyword(lex, T_ELSEIF);
	else if (matches_keyword(lex, "elseifdef")) set_keyword(lex, T_ELSEIFDEF);
	else if (matches_keyword(lex, "export")) set_keyword(lex, T_EXPORT);
	else if (matches_keyword(lex, "fn")) set_keyword(lex, T_FN);
	else if (matches_keyword(lex, "for")) set_keyword(lex, T_FOR);
	else if (matches_keyword(lex, "if")) set_keyword(lex, T_IF);
	else if (matches_keyword(lex, "ifdef")) set_keyword(lex, T_IFDEF);
	else if (matches_keyword(lex, "in")) set_keyword(lex, T_IN);
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

const char *YASL_TOKEN_NAMES[] = {
#define X(E, name, ...) name,
#include "tokentype.x"
#undef X
};

void lex_cleanup(struct Lexer *const lex) {
	lxclose(lex->file);
	io_cleanup(&lex->err);
}
