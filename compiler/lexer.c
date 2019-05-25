#include "lexer.h"

#include "debug.h"
#include "compiler/token.h"
#include "yasl_error.h"
#include "yasl_include.h"

static enum Token YASLToken_ThreeChars(char c1, char c2, char c3);
static enum Token YASLToken_TwoChars(char c1, char c2);
static enum Token YASLToken_OneChar(char c1);
static void YASLKeywords(Lexer *lex);

static int isbdigit(int c) {
	return c == '0' || c == '1';
}

#define isyaslidstart(c) (isalpha(c) || (c) == '_' || (c) == '$')
#define isyaslid(c) (isalnum(c) || (c) == '_' || (c) == '$')

static void lex_error(Lexer *lex) {
	free(lex->value);
	lex->value = NULL;
	lex->type = T_UNKNOWN;
	lex->status = YASL_SYNTAX_ERROR;
}

int lex_getchar(Lexer *lex) {
	return lex->c = lxgetc(lex->file);
}


void lex_rewind(Lexer *lex, int len) {
	lxseek(lex->file, len - 1, SEEK_CUR);
	lex_getchar(lex);
}

static int lex_eatwhitespace(Lexer *lex) {
	while (!lxeof(lex->file) && (lex->c == ' ' || lex->c == '\n' || lex->c == '\t')) {
		if (lex->c == '\n') {
			lex->line++;
			if (ispotentialend(lex)) {
				lex->type = T_SEMI;
				return 1;
			}
		}
		lex_getchar(lex);
	}
	return 0;
}

static int lex_eatinlinecomments(Lexer *lex) {
	if ('#' == lex->c) while (!lxeof(lex->file) && lex_getchar(lex) != '\n') {}
	return 0;
}

static int lex_eatcommentsandwhitespace(Lexer * lex) {
	while ((!lxeof(lex->file) && (lex->c == ' ' || lex->c == '\n' || lex->c == '\t')) || lex->c == '#' ||
	       lex->c == '/') {
		// white space
		if (lex_eatwhitespace(lex)) return 1;

		// inline comments
		if (lex_eatinlinecomments(lex)) return 1;

		// block comments
		if (lex->c == '/') {
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
					YASL_PRINT_ERROR_SYNTAX("Unclosed block comment in line %zd.\n", lex->line);
					lex_error(lex);
					return 1;
				}
				if (addsemi && ispotentialend(lex)) {
					lex->type = T_SEMI;
					return 1;
				}
			} else if (lxeof(lex->file)) {
				lex->type = T_SLASH;
				return 1;
			} else {
				lex_rewind(lex, -1);
				break;
			}
		}
	}
	return 0;
}

static int lex_eatint(Lexer *lex, char separator, int (*isvaliddigit)(int)) {
	size_t i = 0;
	lex->value[i++] = '0';
	lex->value[i++] = separator;
	lex_getchar(lex);

	// eat leading newlines for literals
	while (lex->c == NUM_SEPERATOR) lex_getchar(lex);

	if (!(*isvaliddigit)(lex->c)) {
		YASL_PRINT_ERROR_SYNTAX("Invalid int literal in line %zd.\n", lex->line);
		lex_error(lex);
		return 1;
	}

	do {
		lex->value[i++] = lex->c;
		lex_getchar(lex);
		if (i == lex->val_len) {
			lex->val_len *= 2;
			lex->value = (char *)realloc(lex->value, lex->val_len);
		}
		while (lex->c == NUM_SEPERATOR) lex_getchar(lex);
	} while (!lxeof(lex->file) && (*isvaliddigit)(lex->c));
	if (i == lex->val_len) lex->value = (char *)realloc(lex->value, i + 1);
	lex->value[i] = '\0';
	lex->type = T_INT;
	if (!lxeof(lex->file)) lxseek(lex->file, -1, SEEK_CUR);
	return 1;
}

/*
static int lex_eatnotin(Lexer *lex) {
	char c1, c2, c3, c4;
	c1 = lex->c;
	if (c1 != '!' || lxeof(lex->file)) {
		lex_rewind(lex, -1);
		return 0;
	}

	c2 = lex_getchar(lex);
	if (c2 != 'i' || lxeof(lex->file)) {
		lex_rewind(lex, -2);
		return 0;
	}

	c3 = lex_getchar(lex);
	if (c3 !=  'n' || lxeof(lex->file)) {
		lex_rewind(lex, -3);
		return 0;
	}

	c4 = lex_getchar(lex);
	if (isyaslid(c4) || lxeof(lex->file)) {
		lex_rewind(lex, -4);
		return 0;
	}

	lex->type = T_BANGIN;
	return 1;



}
*/

static int lex_eatop(Lexer *lex) {
	char c1, c2, c3;
	enum Token last;
	c1 = lex->c;
	c2 = lxgetc(lex->file);
	if (lxeof(lex->file)) {
		goto one;
	}

	c3 = lxgetc(lex->file);
	if (lxeof(lex->file)) {
		goto two;
	}

	last = YASLToken_ThreeChars(c1, c2, c3);
	if (last != -1) {
		lex->type = last;
		free(lex->value); // = realloc(lex->value, 0);
		return 1;
	}
	lxseek(lex->file, -1, SEEK_CUR);

	two:
	last = YASLToken_TwoChars(c1, c2);
	if (last != -1) {
		lex->type = last;
		free(lex->value); // = realloc(lex->value, 0);
		return 1;
	}
	lxseek(lex->file, -1, SEEK_CUR);

	one:
	last = YASLToken_OneChar(c1);
	if (last != -1) {
		lex->type = last;
		free(lex->value); // = realloc(lex->value, 0);
		return 1;
	}
	return 0;
}

int lex_eatnumber(Lexer *lex) {
	int c1 = lex->c;
	if (isdigit(c1)) {                          // numbers
		lex->val_len = 8;
		lex->value = (char *)realloc(lex->value, lex->val_len);
		size_t i = 0;
		int c2 = lxgetc(lex->file);

		// hexadecimal literal
		if (c1 == '0' && (c2 == 'x' || c2 == 'X')) {
			if (lex_eatint(lex, 'x', &isxdigit)) return 1;
		}

		// binary literal
		if (c1 == '0' && (c2 == 'b' || c2 == 'B')) {
			if (lex_eatint(lex, 'b', &isbdigit)) return 1;
		}

		// rewind, because we don't have a hexadecimal or binary number.
		if (!lxeof(lex->file)) lxseek(lex->file, -1, SEEK_CUR);

		// decimal (or first half of float)
		do {
			lex->value[i++] = c1;
			lex_getchar(lex);
			if (i == lex->val_len) {
				lex->val_len *= 2;
				lex->value = (char *)realloc(lex->value, lex->val_len);
			}
			while (lex->c == NUM_SEPERATOR) lex_getchar(lex);
			c1 = lex->c;
		} while (!lxeof(lex->file) && ((isdigit(c1))));

		while (lex->c == NUM_SEPERATOR) lex_getchar(lex);
		lex->type = T_INT;
		if (i == lex->val_len) lex->value = (char *)realloc(lex->value, i + 1);
		lex->value[i] = '\0';

		// floats
		if (c1 == '.') {
			c2 = lxgetc(lex->file);
			if (lxeof(lex->file)) {
				lxseek(lex->file, -1, SEEK_CUR);
				return 1;
			}
			if (!isdigit(c2)) {
				lxseek(lex->file, -2, SEEK_CUR);
				return 1;
			}
			lxseek(lex->file, -1, SEEK_CUR);
			do {
				lex->value[i++] = c1;
				lex_getchar(lex);
				if (i == lex->val_len) {
					lex->val_len *= 2;
					lex->value = (char *)realloc(lex->value, lex->val_len);
				}
				while (lex->c == NUM_SEPERATOR) lex_getchar(lex);
				c1 = lex->c;
			} while (!lxeof(lex->file) && isdigit(c1));

			if (lex->c == 'e' || lex->c == 'E') {
				lex_getchar(lex);
				lex->value[i++] = 'e';
				while (lex->c == NUM_SEPERATOR) lex_getchar(lex);
				c1 = lex->c;
				do {
					lex->value[i++] = c1;
					lex_getchar(lex);
					if (i == lex->val_len) {
						lex->val_len *= 2;
						lex->value = (char *)realloc(lex->value, lex->val_len);
					}
					while (lex->c == NUM_SEPERATOR) lex_getchar(lex);
					c1 = lex->c;
				} while (!lxeof(lex->file) && isdigit(c1));
			}

			if (i == lex->val_len) lex->value = (char *)realloc(lex->value, i + 1);
			lex->value[i] = '\0';
			if (!lxeof(lex->file)) lxseek(lex->file, -1, SEEK_CUR);
			lex->type = T_FLOAT;
			return 1;
		}

		if (!lxeof(lex->file)) lxseek(lex->file, -1, SEEK_CUR);
		return 1;
	}
	return 0;
}

int lex_eatid(Lexer *lex) {
	int c = lex->c;
	if (isyaslidstart(c)) {                           // identifiers and keywords
		lex->val_len = 8;
		lex->value = (char *)realloc(lex->value, lex->val_len);
		size_t i = 0;
		do {
			lex->value[i++] = c;
			lex_getchar(lex);
			c = lex->c;
			if (i == lex->val_len) {
				lex->val_len *= 2;
				lex->value = (char *)realloc(lex->value, lex->val_len);
			}
		} while (!lxeof(lex->file) && isyaslid(c));
		if (!lxeof(lex->file)) lxseek(lex->file, -1, SEEK_CUR);
		lex->value = (char *)realloc(lex->value, 1 + (lex->val_len = i));
		lex->value[lex->val_len] = '\0';

		if (lex->type == T_DOT || lex->type == T_RIGHT_ARR) {
			lex->type = T_ID;
			return 1;
		}

		lex->type = T_ID;
		YASLKeywords(lex);                  // keywords
		return 1;
	}
	return 0;
}

int handle_escapes(Lexer *lex, size_t *i, char delim) {
	char buffer[9];
	char tmp;
	char *end;
	switch ((lex)->c) {
	case 'a':
		(lex)->value[(*i)++] = '\a';
		break;
	case 'b':
		(lex)->value[(*i)++] = '\b';
		break;
	case 'f':
		(lex)->value[(*i)++] = '\f';
		break;
	case 'n':
		(lex)->value[(*i)++] = '\n';
		break;
	case 'r':
		(lex)->value[(*i)++] = '\r';
		break;
	case 't':
		(lex)->value[(*i)++] = '\t';
		break;
	case 'v':
		(lex)->value[(*i)++] = '\v';
		break;
	case '0':
		(lex)->value[(*i)++] = '\0';
		break;
	case STR_DELIM:
		(lex)->value[(*i)++] = STR_DELIM;
		break;
	case INTERP_STR_DELIM:
		(lex)->value[(*i)++] = INTERP_STR_DELIM;
		break;
	case INTERP_STR_PLACEHOLDER:
		(lex)->value[(*i)++] = INTERP_STR_PLACEHOLDER;
		break;
	case ESCAPE_CHAR:
		(lex)->value[(*i)++] = ESCAPE_CHAR;
		break;
	case 'x':
		buffer[0] = lex_getchar(lex);
		buffer[1] = lex_getchar(lex);
		buffer[2] = '\0';
		tmp = (char) strtol(buffer, &end, 16);
		if (end != buffer + 2) {
			YASL_PRINT_ERROR_SYNTAX("Invalid hex string escape in line %zd.\n", lex->line);
			while (lex->c != '\n' && lex->c != delim) lex_getchar(lex);
			lex_error(lex);
			return 1;
		}
		lex->value[(*i)++] = tmp;
		return 0;
	default:
		YASL_PRINT_ERROR_SYNTAX("Invalid string escape sequence in line %zd.\n", (lex)->line);
		while (lex->c != '\n' && lex->c != delim) lex_getchar(lex);
		lex_error(lex);
		return 1;
	}
	return 0;
}

int lex_eatinterpstringbody(Lexer *lex) {
	lex->val_len = 6;
	lex->value = (char *)realloc(lex->value, lex->val_len);
	size_t i = 0;
	lex->type = T_STR;

	while (lex->c != INTERP_STR_DELIM && lex->c != INTERP_STR_PLACEHOLDER && !lxeof(lex->file)) {
		if (lex->c == '\n') {
			YASL_PRINT_ERROR_SYNTAX("Unclosed string literal in line %zd.\n", lex->line);
			lex_error(lex);
			return 1;
		}

		if (lex->c == ESCAPE_CHAR) {
			lex_getchar(lex);
			if (handle_escapes(lex, &i, INTERP_STR_DELIM)) return 1;
		} else {
			lex->value[i++] = lex->c;
		}

		lex_getchar(lex);

		if (i == lex->val_len) {
			lex->val_len *= 2;
			lex->value = (char *)realloc(lex->value, lex->val_len);
		}
	}

	if (lex->c == INTERP_STR_PLACEHOLDER) {
		lex->mode = L_INTERP;
	} else {
		lex->mode = L_NORMAL;
	}

	lex->val_len = i;

	if (lxeof(lex->file)) {
		YASL_PRINT_ERROR_SYNTAX("Unclosed string literal in line %zd.\n", lex->line);
		lex_error(lex);
		return 1;
	}

	return 1;
}

int lex_eatinterpstring(Lexer *lex) {
	if (lex->c == INTERP_STR_DELIM) {
		lex->val_len = 8;
		lex->value = (char *)realloc(lex->value, lex->val_len);
		size_t i = 0;
		lex->type = T_STR;

		lex_getchar(lex);
		while (lex->c != INTERP_STR_DELIM && lex->c != INTERP_STR_PLACEHOLDER && !lxeof(lex->file)) {
			if (lex->c == '\n') {
				YASL_PRINT_ERROR_SYNTAX("Unclosed string literal in line %zd.\n", lex->line);
				lex_error(lex);
				return 1;
			}

			if (lex->c == ESCAPE_CHAR) {
				lex_getchar(lex);
				if (handle_escapes(lex, &i, INTERP_STR_DELIM)) return 1;
			} else {
				lex->value[i++] = lex->c;
			}

			lex_getchar(lex);

			if (i == lex->val_len) {
				lex->val_len *= 2;
				lex->value = (char *)realloc(lex->value, lex->val_len);
			}
		}

		if (lex->c == INTERP_STR_PLACEHOLDER) {
			lex->mode = L_INTERP;
		} else {
			lex->mode = L_NORMAL;
		}

		lex->val_len = i;

		if (lxeof(lex->file)) {
			YASL_PRINT_ERROR_SYNTAX("Unclosed string literal in line %zd.\n", lex->line);
			lex_error(lex);
			return 1;
		}

		return 1;

	}
	return 0;
}

static int lex_eatstring(Lexer *lex) {
	if (lex->c == STR_DELIM) {
		lex->val_len = 6;
		lex->value = (char *)realloc(lex->value, lex->val_len);
		size_t i = 0;
		lex->type = T_STR;

		lex_getchar(lex);
		while (lex->c != STR_DELIM && !lxeof(lex->file)) {
			if (lex->c == '\n') {
				YASL_PRINT_ERROR_SYNTAX("Unclosed string literal in line %zd.\n", lex->line);
				lex_error(lex);
				return 1;
			}

			if (lex->c == ESCAPE_CHAR) {
				lex_getchar(lex);
				if (handle_escapes(lex, &i, STR_DELIM)) {
					return 1;
				}
			} else {
				lex->value[i++] = lex->c;
			}
			lex_getchar(lex);
			if (i == lex->val_len) {
				lex->val_len *= 2;
				lex->value = (char *)realloc(lex->value, lex->val_len);
			}
		}

		lex->val_len = i;

		if (lxeof(lex->file)) {
			YASL_PRINT_ERROR_SYNTAX("Unclosed string literal in line %zd.\n", lex->line);
			lex_error(lex);
			return 1;
		}

		return 1;

	}
	return 0;
}


static int lex_eatrawstring(Lexer *lex) {
	if (lex->c == RAW_STR_DELIM) {
		lex->val_len = 8;
		lex->value = (char *)realloc(lex->value, lex->val_len);
		size_t i = 0;
		lex->type = T_STR;

		lex_getchar(lex);
		while (lex->c != RAW_STR_DELIM && !lxeof(lex->file)) {
			if (lex->c == '\n') lex->line++;
			lex->value[i++] = lex->c;
			lex_getchar(lex);
			if (i == lex->val_len) {
				lex->val_len *= 2;
				lex->value = (char *)realloc(lex->value, lex->val_len);
			}
		}

		lex->val_len = i;

		if (lxeof(lex->file)) {
			YASL_PRINT_ERROR_SYNTAX("Unclosed string literal in line %zd.\n", lex->line);
			lex_error(lex);
			return 1;
		}

		return 1;

	}
	return 0;
}

void gettok(Lexer *lex) {
	YASL_LEX_DEBUG_LOG("getting token from line %zd\n", lex->line);
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

	// !in operator
	//if (lex_eatnotin(lex)) return;

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

	YASL_PRINT_ERROR_SYNTAX("Unknown character in line %zd: `%c` (0x%x).\n", lex->line, lex->c, lex->c);
	lex_error(lex);
}

static enum Token YASLToken_ThreeChars(char c1, char c2, char c3) {
	switch(c1) {
	case '<': switch(c2) { case '<': switch(c3) { case '=': return T_DLTEQ;} } return T_UNKNOWN;
	case '>': switch(c2) { case '>': switch(c3) { case '=': return T_DGTEQ; } } return T_UNKNOWN;
	case '=': switch(c2) { case '=': switch(c3) { case '=': return T_TEQ; } } return T_UNKNOWN;
		case '!': switch(c2) {
			case '=': switch(c3) { case '=': return T_BANGDEQ;}
			  return T_UNKNOWN;
			// case 'i': switch(c3) { case 'n': return T_BANGIN; default: return T_UNKNOWN; }
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
	case ':': switch(c2) { case '=': return T_COLONEQ;} return T_UNKNOWN;
	case '^': switch(c2) { case '=': return T_CARETEQ;} return T_UNKNOWN;
	case '+': switch(c2) { case '=': return T_PLUSEQ;} return T_UNKNOWN;
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
	case '~': switch(c2) { case '=': return T_TILDEEQ;} return T_UNKNOWN;
		case '*': switch(c2) {
			case '=': return T_STAREQ;
			case '*': return T_DSTAR;
	  } return T_UNKNOWN;
		case '/': switch(c2) {
				case '=': return T_SLASHEQ;
	  case '/': return T_DSLASH;} return T_UNKNOWN;
	case '%': switch(c2) { case '=': return T_MODEQ;} return T_UNKNOWN;
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
	case '?': switch(c2) { case '?': return T_DQMARK;} return T_UNKNOWN;
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

static int matches_keyword(Lexer *lex, const char *string) {
	return strlen(string) == lex->val_len && !memcmp(lex->value, string, lex->val_len);
}

static void set_keyword(Lexer *lex, enum Token type) {
	lex->type = type;
	free(lex->value);
	lex->value = NULL;
}

static void YASLKeywords(Lexer *lex) {
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
	 */

	if (matches_keyword(lex, "enum")) {
		YASL_PRINT_ERROR_SYNTAX("enum is an unused reserved word and cannot be used (line %zd).\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "yield")) {
		YASL_PRINT_ERROR_SYNTAX("yield is an unused reserved word and cannot be used (line %zd).\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "do")) {
		YASL_PRINT_ERROR_SYNTAX("do is an unused reserved word and cannot be used (line %zd).\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "use")) {
		YASL_PRINT_ERROR_SYNTAX("use is an unused reserved word and cannot be used (line %zd).\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "no")) {
		YASL_PRINT_ERROR_SYNTAX("no is an unused reserved word and cannot be used (line %zd).\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "pure")) {
		YASL_PRINT_ERROR_SYNTAX("yield is an unused reserved word and cannot be used (line %zd).\n", lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "consteval")) {
		YASL_PRINT_ERROR_SYNTAX("consteval is an unused reserved word and cannot be used (line %zd).\n",
					lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "constexpr")) {
		YASL_PRINT_ERROR_SYNTAX("constexpr is an unused reserved word and cannot be used (line %zd).\n",
					lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "constfold")) {
		YASL_PRINT_ERROR_SYNTAX("constfold is an unused reserved word and cannot be used (line %zd).\n",
					lex->line);
		lex_error(lex);
		return;
	} else if (matches_keyword(lex, "extern")) {
		YASL_PRINT_ERROR_SYNTAX("extern is an unused reserved word and cannot be used (line %zd).\n",
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
	else if (matches_keyword(lex, "in")) set_keyword(lex, T_IN);
	else if (matches_keyword(lex, "echo")) set_keyword(lex, T_ECHO);
	else if (matches_keyword(lex, "let")) set_keyword(lex, T_LET);
	else if (matches_keyword(lex, "return")) set_keyword(lex, T_RET);
	else if (matches_keyword(lex, "undef")) set_keyword(lex, T_UNDEF);
	else if (matches_keyword(lex, "while")) set_keyword(lex, T_WHILE);
	else if (matches_keyword(lex, "len")) set_keyword(lex, T_LEN);
		// NOTE: special case for bools and floats
		// else if (matches_keyword(lex, "nan") || matches_keyword(lex, "inf")) lex->type = T_FLOAT;
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
	"in",           // T_IN
	"!in",          // T_BANGIN,
	"id",           // T_ID,
	"const",        // T_CONST,
	"fn",           // T_FN,
	"let",          // T_LET,
	"return",       // T_RET,
	"export",         // T_EXPORT,
	"echo",         // T_PRINT,
	"len",
	"(",            // LPAR,
	")",            // RPAR,
	"[",            // LSQB,
	"]",            // RSQB,
	"{",            // LBRC,
	"}",            // RBRC,
	".",            // DOT,
	",",            // COMMA,
	"^",            // CARET,
	"^=",           //CARETEQ,
	"+",            // PLUS,
	"+=",           // PLUSEQ,
	"-",            // MINUS,
	"-=",           // MINUSEQ,
	"#",            // HASH,
	"@",            // AT,
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

Lexer *lex_new(FILE *file /* OWN */) {
	Lexer *lex = (Lexer *) malloc(sizeof(Lexer));
	lex->line = 1;
	lex->value = NULL;
	lex->val_len = 0;
	lex->file = lexinput_new_file(file);
	lex->type = T_UNKNOWN;
	lex->status = YASL_SUCCESS;
	lex->mode = L_NORMAL;
	return lex;
}

void lex_cleanup(Lexer *lex) {
	lxclose(lex->file);
}
