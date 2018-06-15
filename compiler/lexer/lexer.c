#include <debug.h>
#include "lexer.h"
#include "../token.h"

char lex_getchar(Lexer *lex) {
    return lex->c = fgetc(lex->file);
}

//void lex_eatinlinecomments(Lexer *lex) {
//    while (!feof(lex->file) && lex_getchar(lex) != '\n') {}
//}

void gettok(Lexer *lex) {
    YASL_TRACE_LOG("getting token from line %d\n", lex->line);
    char c1, c2, c3, c4;
    int last;
    lex_getchar(lex);
    c1 = lex->c;

    // hash-bang
    if (lex->line == 1 && c1 == '#') {
        lex_getchar(lex);
        c1 = lex->c;
        if (c1 == '!') {
            while (!feof(lex->file) && fgetc(lex->file) != '\n') {}
        } else {
            fseek(lex->file, -2, SEEK_CUR);
        }
        lex_getchar(lex);
        c1 = lex->c;
    }

    // whitespace and comments.
    while (!feof(lex->file) && (c1 == ' ' || c1 == '\n' || c1 == '\t') || c1 == '"' || c1 == '/') {
        // white space
        while (!feof(lex->file) && (c1 == ' ' || c1 == '\n' || c1 == '\t')) {
            if (c1 == '\n') {
                lex->line++;
                if (ispotentialend(lex)) {
                    lex->type = T_SEMI;
                    return;
                }
            }
            lex_getchar(lex);
            c1 = lex->c;
        }

        // inline comments
        if ('"' == lex->c) while (!feof(lex->file) && lex_getchar(lex) != '\n') {}
        c1 = lex->c;

        // block comments
        if (c1 == '/') {
            lex_getchar(lex);
            c1 = lex->c;
            if (c1 == '*') {
                int addsemi = 0;
                lex_getchar(lex);
                c1 = lex->c;
                c2 = fgetc(lex->file);
                while (!feof(lex->file) && (c1 != '*' || c2 != '/')) {
                    if (c1 == '\n' || c2 == '\n') addsemi = 1;
                    if (c1 == '\n') lex->line++;
                    c1 = c2;
                    c2 = fgetc(lex->file);
                }
                if (feof(lex->file)) {
                    puts("LexingError: unclosed block comment.");
                    exit(EXIT_FAILURE);
                }
                if (addsemi && ispotentialend(lex)) {
                    lex->type = T_SEMI;
                    return;
                }
            } else {
                fseek(lex->file, -2, SEEK_CUR);
            }
            lex_getchar(lex);
            c1 = lex->c;
        }
    }

    // EOF
    if (feof(lex->file)) {
        lex->type = T_EOF;
        lex->value = NULL;
        return;
    }

    // numbers
    if (isdigit(c1)) {                          // numbers
        lex->val_len = 6;
        lex->value = realloc(lex->value, lex->val_len);
        int i = 0;
        c2 = fgetc(lex->file);

        // hex literal
        if (c1 == '0' && c2 == 'x'){            // hexadecimal literal
            lex->value[i++] = '0';
            lex->value[i++] = 'x';
            lex_getchar(lex);
            c1 = lex->c;
            do {
                lex->value[i++] = c1;
                lex_getchar(lex);
                c1 = lex->c;
                if (i == lex->val_len) {
                    lex->val_len *= 2;
                    lex->value = realloc(lex->value, lex->val_len);
                }
            } while (!feof(lex->file) && isxdigit(c1));    // isxdigit checks if a hex digit.
            if (i == lex->val_len) lex->value = realloc(lex->value, i + 1);
            lex->value[i] = '\0';
            lex->type = T_INT64;
            if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
            return;
        }

        // binary literal
        if (c1 == '0' && c2 == 'b') {
            lex->value[i++] = '0';
            lex->value[i++] = 'b';
            lex_getchar(lex);
            c1 = lex->c;
            do {
                lex->value[i++] = c1;
                lex_getchar(lex);
                c1 = lex->c;
                if (i == lex->val_len) {
                    lex->val_len *= 2;
                    lex->value = realloc(lex->value, lex->val_len);
                }
            } while (!feof(lex->file) && isbdigit(c1));    // isbdigit checks if a binary digit ('1' or '0').
            if (i == lex->val_len) lex->value = realloc(lex->value, i + 1);
            lex->value[i] = '\0';
            lex->type = T_INT64;
            if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
            return;
        }

        // octal literal
        if (c1 == '0' && c2 == 'o') {
            lex->value[i++] = '0';
            lex->value[i++] = 'o';
            lex_getchar(lex);
            c1 = lex->c;
            do {
                lex->value[i++] = c1;
                lex_getchar(lex);
                c1 = lex->c;
                if (i == lex->val_len) {
                    lex->val_len *= 2;
                    lex->value = realloc(lex->value, lex->val_len);
                }
            } while (!feof(lex->file) && isodigit(c1));    // isodigit checks if an octal digit.
            if (i == lex->val_len) lex->value = realloc(lex->value, i + 1);
            lex->value[i] = '\0';
            lex->type = T_INT64;
            if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
            return;
        }

        // rewind, because we don't have an octal, hex, or binary number.
        if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);

        // decimal (or first half of float)
        do {
            lex->value[i++] = c1;
            lex_getchar(lex);
            c1 = lex->c;
            if (i == lex->val_len) {
                lex->val_len *= 2;
                lex->value = realloc(lex->value, lex->val_len);
            }
        } while (!feof(lex->file) && isdigit(c1));
        lex->type = T_INT64;
        if (i == lex->val_len) lex->value = realloc(lex->value, i + 1);
        lex->value[i] = '\0';

        // floats
        if (c1 == '.') {
            c2 = fgetc(lex->file);
            if (feof(lex->file)) {
                fseek(lex->file, -1, SEEK_CUR);
                return;
            }
            if (!isdigit(c2)) {
                fseek(lex->file, -2, SEEK_CUR);
                return;
            }
            fseek(lex->file, -1, SEEK_CUR);
            do {
                lex->value[i++] = c1;
                lex_getchar(lex);
                c1 = lex->c;
                if (i == lex->val_len) {
                    lex->val_len *= 2;
                    lex->value = realloc(lex->value, lex->val_len);
                }
            } while (!feof(lex->file) && isdigit(c1));

            if (i == lex->val_len) lex->value = realloc(lex->value, i + 1);
            lex->value[i] = '\0';
            if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
            lex->type = T_FLOAT64;
            return;
        }

        if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
        return;
    }

    // identifiers and keywords
    if (isalpha(c1)) {                           // identifiers and keywords
        lex->val_len = 6;
        lex->value = realloc(lex->value, lex->val_len);
        int i = 0;
        do {
            lex->value[i++] = c1;
            lex_getchar(lex);
            c1 = lex->c;
            if (i == lex->val_len) {
                lex->val_len *= 2;
                lex->value = realloc(lex->value, lex->val_len);
            }
        } while (!feof(lex->file) && isalnum(c1));
        if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
        lex->value = realloc(lex->value, lex->val_len = i);
        lex->type = T_ID;
        YASLKeywords(lex);          // keywords
        return;
    }

    // strings
    if (c1 == STR_DELIM) {                             // strings
        lex->val_len = 6;
        lex->value = realloc(lex->value, lex->val_len);
        int i = 0;
        lex_getchar(lex);
        c1 = lex->c;
        while (c1 != STR_DELIM && !feof(lex->file)) {
            lex->value[i++] = c1;
            lex_getchar(lex);
            c1 = lex->c;
            if (i == lex->val_len) {
                lex->val_len *= 2;
                lex->value = realloc(lex->value, lex->val_len);
            }
        }
        lex->value = realloc(lex->value, lex->val_len = i);

        if (feof(lex->file)) {
            puts("LexingError: unclosed string literal.");
            exit(EXIT_FAILURE);
        }
        lex->type = T_STR;
        return;
    }

    // operators
    c2 = fgetc(lex->file);
    if (feof(lex->file)) {
        goto one;
    }

    c3 = fgetc(lex->file);
    if (feof(lex->file)) {
        goto two;
    }

    c4 = fgetc(lex->file);
    if (feof(lex->file)) {
        goto three;
    }

    four:
    last = YASLToken_FourChars(c1, c2, c3, c4);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }
    fseek(lex->file, -1, SEEK_CUR);

    three:
    last = YASLToken_ThreeChars(c1, c2, c3);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }
    fseek(lex->file, -1, SEEK_CUR);

    two:
    last = YASLToken_TwoChars(c1, c2);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }
    fseek(lex->file, -1, SEEK_CUR);

    one:
    last = YASLToken_OneChar(c1);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }

    printf("LexingError: unknown lexeme in line %d: `%c` (0x%x)\n", lex->line, c1, c1);
    exit(EXIT_FAILURE);
}

Token YASLToken_FourChars(char c1, char c2, char c3, char c4) {
    switch(c1) { case '|': switch(c2) { case '|': switch(c3) { case '|':  switch(c4) { case '=': return T_TBAREQ; default: return T_UNKNOWN;} } } }
    return T_UNKNOWN;
}

Token YASLToken_ThreeChars(char c1, char c2, char c3) {
    switch(c1) {
        case '<': switch(c2) { case '<': switch(c3) { case '=': return T_DLTEQ; default: return T_UNKNOWN;} }
        case '>': switch(c2) { case '>': switch(c3) { case '=': return T_DGTEQ; default: return T_UNKNOWN;} }
        case '=': switch(c2) { case '=': switch(c3) { case '=': return T_TEQ; default: return T_UNKNOWN;} }
        case '!': switch(c2) { case '=': switch(c3) { case '=': return T_BANGDEQ; default: return T_UNKNOWN;} }
        case '*': switch(c2) { case '*': switch(c3) { case '=': return T_DSTAREQ; default: return T_UNKNOWN;} }
        case '/': switch(c2) { case '/': switch(c3) { case '=': return T_DSLASHEQ; default: return T_UNKNOWN;} }
        case '|': switch(c2) { case '|': switch(c3) {
                        case '=': return T_DBAREQ;
                        case '|': return T_TBAR;
                        default: return T_UNKNOWN;
        } }
        case '?': switch(c2) { case '?': switch(c3) { case '=': return T_DQMARKEQ; default: return T_UNKNOWN;} }
    }
    return T_UNKNOWN;
}

Token YASLToken_TwoChars(char c1, char c2) {
    switch(c1) {
        case '^': switch(c2) { case '=': return T_CARETEQ; default: return T_UNKNOWN; };
        case '+': switch(c2) { case '=': return T_PLUSEQ; default: return T_UNKNOWN; };
        case '-': switch(c2) {
                case '=': return T_MINUSEQ;
                case '>': return T_RARR;
                default: return T_UNKNOWN;
        }
        case '=': switch(c2) { case '=': return T_DEQ; default: return T_UNKNOWN;}
        case '!': switch(c2) { case '=': return T_BANGEQ; default: return T_UNKNOWN;}
        case '~': switch(c2) { case '=': return T_TILDEEQ; default: return T_UNKNOWN;}
        case '*': switch(c2) {
            case '=': return T_STAREQ;
            case '*': return T_DSTAR;
            default: return T_UNKNOWN;
        }
        case '/': switch(c2) {
                case '=': return T_SLASHEQ;
                case '/': return T_DSLASH;
                default: return T_UNKNOWN; }
        case '%': switch(c2) { case '=': return T_MODEQ; default: return T_UNKNOWN;}
        case '<': switch(c2) {
                case '=': return T_LTEQ;
                case '<': return T_DLT;
                case '-': return T_LARR;
                default: return T_UNKNOWN;
        }
        case '>': switch(c2) {
                case '=': return T_GTEQ;
                case '>': return T_DGT;
                default:  return T_UNKNOWN;
        }
        case '&': switch(c2) { case '=': return T_AMPEQ; default: return T_UNKNOWN; }
        case '|': switch(c2) {
                case '=': return T_BAREQ;
                case '|': return T_DBAR;
                default: return T_UNKNOWN;
        }
        case '?': switch(c2) { case '?': return T_DQMARK; default: return T_UNKNOWN;}
    }
    return T_UNKNOWN;
}

Token YASLToken_OneChar(char c1) {
    switch(c1) {
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
        case '#': return T_HASH;
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

int max(int a, int b) {
    return a > b ? a : b;
}

void YASLKeywords(Lexer *lex) {
    /* keywords:
     *  let
     *  print
     *  if
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

    if (strlen("let") == lex->val_len && !memcmp(lex->value, "let", lex->val_len)) {
        lex->type = T_LET;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("print") == lex->val_len && !memcmp(lex->value, "print", lex->val_len)) {
        lex->type = T_PRINT;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("else") == lex->val_len && !memcmp(lex->value, "else", lex->val_len)) {
        lex->type = T_ELSE;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("if") == lex->val_len && !memcmp(lex->value, "if", lex->val_len)) {
        lex->type = T_IF;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("elseif") == lex->val_len && !memcmp(lex->value, "elseif", lex->val_len)) {
        lex->type = T_ELSEIF;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("while") == lex->val_len && !memcmp(lex->value, "while", lex->val_len)) {
        lex->type = T_WHILE;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("for") == lex->val_len && !memcmp(lex->value, "for", lex->val_len)) {
        lex->type = T_FOR;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("break") == lex->val_len && !memcmp(lex->value, "break", lex->val_len)) {
        lex->type = T_BREAK;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("continue") == lex->val_len && !memcmp(lex->value, "continue", lex->val_len)) {
        lex->type = T_CONT;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("true") == lex->val_len && !memcmp(lex->value, "true", lex->val_len)) {
        lex->type = T_BOOL;
    } else if (strlen("false") == lex->val_len && !memcmp(lex->value, "false", lex->val_len)) {
        lex->type = T_BOOL;
    } else if (strlen("or") == lex->val_len && !memcmp(lex->value, "or", lex->val_len)) {
        lex->type = T_OR;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("and") == lex->val_len && !memcmp(lex->value, "and", lex->val_len)) {
        lex->type = T_AND;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("undef") == lex->val_len && !memcmp(lex->value, "undef", lex->val_len)) {
        lex->type = T_UNDEF;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("fn") == lex->val_len && !memcmp(lex->value, "fn", lex->val_len)) {
        lex->type = T_FN;
        lex->value = realloc(lex->value, 0);
    } else if (strlen("return") == lex->val_len && !memcmp(lex->value, "return", lex->val_len)) {
        lex->type = T_RET;
        lex->value = realloc(lex->value, 0);
    }
}

const char *YASL_TOKEN_NAMES[] = {
        "END OF FILE",  // T_EOF,
        ";",            // T_SEMI,
        "undef",        // T_UNDEF,
        "float64",      // T_FLOAT64,
        "int64",        // T_INT64,
        "bool",         // T_BOOL,
        "str",          // T_STR,
        "if",           // T_IF,
        "elseif",       // T_ELSEIF,
        "else",         // T_ELSE,
        "while",        // T_WHILE,
        "break",        // T_BREAK,
        "continue",     // T_CONT,
        "for",          // T_FOR,
        "and",          // T_AND,
        "or",           // T_OR,
        "id",           // T_ID,
        "let",          // T_LET,
        "fn",           // T_FN,
        "return",       // T_RET,
        "enum",         // T_ENUM,
        "struct",       // T_STRUCT,
        "print",        // T_PRINT,
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
        "!",            // BANG,
        "!=",           // BANGEQ,
        "!==",          // BANGDEQ,
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
        "&",            // AMP,
        "&=",           // AMPEQ,
        "|",            // BAR,
        "|=",           // BAREQ,
        "||",           // DBAR,
        "||=",          // DBAREQ,
        "|||",          // TBAR,
        "|||=",         // TBAREQ,
        "?",            // QMARK,
        "??",           // DQMARK,
        "?\?=",         // DQMARKEQ,
        ":",            // COLON,
        "->",           // RARR,
        "<-",           // LARR,
};

Lexer *lex_new(FILE *file) {
    Lexer *lex = malloc(sizeof(Lexer));
    lex->line = 1;
    lex->value = NULL;
    lex->val_len = 0;
    lex->file = file;
    lex->type = T_UNKNOWN;
    return lex;
}

void lex_del(Lexer *lex) {
    fclose(lex->file);
    free(lex->value);
    free(lex);
}

/*
int main(void) {
    FILE *fp = fopen("sample.yasl", "r");
    if (fp == NULL) return 1;
    fseek(fp, 0, SEEK_SET);
    Lexer *lex = lex_new(fp);
    while (lex->type != T_EOF) {
        gettok(lex);
        printf("type: %s, value: %s\n", YASL_TOKEN_NAMES[lex->type], lex->value);
    }
    lex_del(lex);
}
*/