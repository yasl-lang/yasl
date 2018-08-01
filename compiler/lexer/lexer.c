#include <debug.h>
#include "lexer.h"
#include "../token.h"

static int isbdigit(int c) {
    return c == '0' || c == '1';
}

static int isodigit(int c) {
    return '0' <= c && c < '8';
}

char lex_getchar(Lexer *lex) {
    return lex->c = fgetc(lex->file);
}


char lex_rewind(Lexer *lex, int len) {
    fseek(lex->file, len-1, SEEK_CUR);
    lex_getchar(lex);
}

static int lex_eatwhitespace(Lexer *lex) {
    while (!feof(lex->file) && (lex->c == ' ' || lex->c == '\n' || lex->c == '\t')) {
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
    if ('"' == lex->c) while (!feof(lex->file) && lex_getchar(lex) != '\n') {}
    return 0;
}

static int lex_eatint(Lexer *lex, char separator, int (*isvaliddigit)(int)) {
    size_t i = 0;
    lex->value[i++] = '0';
    lex->value[i++] = separator;
    lex_getchar(lex);
    if (!(*isvaliddigit)(lex->c)) {
        printf("Invalid int64 literal in line %d\n", lex->line);
        exit(EXIT_FAILURE);
    }
    do {
        lex->value[i++] = lex->c;
        lex_getchar(lex);
        if (i == lex->val_len) {
            lex->val_len *= 2;
            lex->value = realloc(lex->value, lex->val_len);
        }
    } while (!feof(lex->file) && (*isvaliddigit)(lex->c));
    if (i == lex->val_len) lex->value = realloc(lex->value, i + 1);
    lex->value[i] = '\0';
    lex->type = T_INT64;
    if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
    return 1;
}

static int lex_eatop(Lexer *lex) {
    char c1, c2, c3;
    int last;
    c1 = lex->c;
    c2 = fgetc(lex->file);
    if (feof(lex->file)) {
        goto one;
    }

    c3 = fgetc(lex->file);
    if (feof(lex->file)) {
        goto two;
    }

    three:
    last = YASLToken_ThreeChars(c1, c2, c3);
    if (last != -1) {
        lex->type = last;
        free(lex->value); // = realloc(lex->value, 0);
        return 1;
    }
    fseek(lex->file, -1, SEEK_CUR);

    two:
    last = YASLToken_TwoChars(c1, c2);
    if (last != -1) {
        lex->type = last;
        free(lex->value); // = realloc(lex->value, 0);
        return 1;
    }
    fseek(lex->file, -1, SEEK_CUR);

    one:
    last = YASLToken_OneChar(c1);
    if (last != -1) {
        lex->type = last;
        free(lex->value); // = realloc(lex->value, 0);
        return 1;
    }
    return 0;
}

static int lex_eatstring(Lexer *lex) {
    if (lex->c == STR_DELIM) {
        lex->val_len = 6;
        lex->value = realloc(lex->value, lex->val_len);
        int i = 0;
        lex->type = T_STR;

        lex_getchar(lex);
        while (lex->c != STR_DELIM && !feof(lex->file)) {
            lex->value[i++] = lex->c;
            lex_getchar(lex);
            if (i == lex->val_len) {
                lex->val_len *= 2;
                lex->value = realloc(lex->value, lex->val_len);
            }
        }
        //lex_getchar(lex);
        lex->value = realloc(lex->value, lex->val_len = i);

        if (feof(lex->file)) {
            puts("LexingError: unclosed string literal.");
            exit(EXIT_FAILURE);
        }

        return 1;

    }
    return 0;
}

void gettok(Lexer *lex) {
    YASL_TRACE_LOG("getting token from line %d\n", lex->line);
    lex->value = NULL;
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
            lex->line++;
        } else if (feof(lex->file)) {
            lex->type = T_HASH;
            return;
        } else {
            fseek(lex->file, -2, SEEK_CUR);
        }
        lex_getchar(lex);
        c1 = lex->c;
    }

    // whitespace and comments.
    while (!feof(lex->file) && (lex->c == ' ' || lex->c == '\n' || lex->c == '\t') || lex->c == '"' || lex->c == '/') {
        // white space
        if (lex_eatwhitespace(lex)) return;

        // inline comments
        if (lex_eatinlinecomments(lex)) return;

        // block comments
        if (lex->c == '/') {
            lex_getchar(lex);
            if (lex->c == '*') {
                int addsemi = 0;
                lex->c = ' ';
                c1 = fgetc(lex->file);
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
            } else if (feof(lex->file)) {
                lex->type = T_SLASH;
                return;
            } else {
                lex_rewind(lex, -1);
                break;
            }
        }
    }

    c1 = lex->c;

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
        c1 = lex->c;
        c2 = fgetc(lex->file);

        // hex literal
        if (c1 == '0' && c2 == 'x'){            // hexadecimal literal
            if (lex_eatint(lex, 'x', &isxdigit)) return;
        }

        // binary literal
        if (c1 == '0' && c2 == 'b') {
            if (lex_eatint(lex, 'b', &isbdigit)) return;
        }

        // octal literal
        if (c1 == '0' && c2 == 'o') {
            if (lex_eatint(lex, 'o', &isodigit)) return;
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
    if (lex_eatstring(lex)) return;

    // operators
    if (lex_eatop(lex)) return;

    printf("LexingError: unknown lexeme in line %d: `%c` (0x%x)\n", lex->line, lex->c, lex->c);
    exit(EXIT_FAILURE);
}

static Token YASLToken_ThreeChars(char c1, char c2, char c3) {
    switch(c1) {
        case '<': switch(c2) { case '<': switch(c3) { case '=': return T_DLTEQ; default: return T_UNKNOWN;} }
        case '>': switch(c2) { case '>': switch(c3) { case '=': return T_DGTEQ; default: return T_UNKNOWN;} }
        case '=': switch(c2) { case '=': switch(c3) { case '=': return T_TEQ; default: return T_UNKNOWN;} }
        case '!': switch(c2) { case '=': switch(c3) { case '=': return T_BANGDEQ; default: return T_UNKNOWN;} }
        case '*': switch(c2) { case '*': switch(c3) { case '=': return T_DSTAREQ; default: return T_UNKNOWN;} }
        case '/': switch(c2) { case '/': switch(c3) { case '=': return T_DSLASHEQ; default: return T_UNKNOWN;} }
        case '&': switch(c2) { case '&': switch(c3) {
                        case '=': return T_DAMPEQ;
                        default: return T_UNKNOWN;
        } }
        case '|': switch(c2) { case '|': switch(c3) {
            case '=': return T_DBAREQ;
            default: return T_UNKNOWN;
        } }
        case '?': switch(c2) { case '?': switch(c3) { case '=': return T_DQMARKEQ; default: return T_UNKNOWN;} }
    }
    return T_UNKNOWN;
}

static Token YASLToken_TwoChars(char c1, char c2) {
    switch(c1) {
        case '^': switch(c2) { case '=': return T_CARETEQ; default: return T_UNKNOWN; };
        case '+': switch(c2) { case '=': return T_PLUSEQ; default: return T_UNKNOWN; };
        case '-': switch(c2) {
                case '=': return T_MINUSEQ;
                case '>': return T_SMALL_ARR;
                default: return T_UNKNOWN;
        }
        case '=': switch(c2) {
            case '=': return T_DEQ;
            case '>': return T_BIG_ARR;
            case '~': return T_EQTILDE;
            default: return T_UNKNOWN;
        }
        case '!': switch(c2) {
            case '=': return T_BANGEQ;
            case '~': return T_BANGTILDE;
            default: return T_UNKNOWN;
        }
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
                default: return T_UNKNOWN;
        }
        case '>': switch(c2) {
                case '=': return T_GTEQ;
                case '>': return T_DGT;
                default:  return T_UNKNOWN;
        }
        case '&': switch(c2) {
            case '=': return T_AMPEQ;
            case '&': return T_DAMP;
            default: return T_UNKNOWN;
        }
        case '|': switch(c2) {
                case '=': return T_BAREQ;
                case '|': return T_DBAR;
                default: return T_UNKNOWN;
        }
        case ':': switch(c2) { case ':': return T_DCOLON; default: return T_UNKNOWN; }
        case '?': switch(c2) { case '?': return T_DQMARK; default: return T_UNKNOWN;}
    }
    return T_UNKNOWN;
}

static Token YASLToken_OneChar(char c1) {
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

static int matches_keyword(Lexer *lex, char *string) {
    return strlen(string) == lex->val_len && !memcmp(lex->value, string, lex->val_len);
}

static void set_keyword(Lexer *lex, Token type) {
    lex->type = type;
    free(lex->value);
}

static void YASLKeywords(Lexer *lex) {
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



    if (matches_keyword(lex, "break")) set_keyword(lex, T_BREAK);
    else if (matches_keyword(lex, "const")) set_keyword(lex, T_CONST);
    else if (matches_keyword(lex, "continue")) set_keyword(lex, T_CONT);
    else if (matches_keyword(lex, "else")) set_keyword(lex, T_ELSE);
    else if (matches_keyword(lex, "elseif")) set_keyword(lex, T_ELSEIF);
    else if (matches_keyword(lex, "fn")) set_keyword(lex, T_FN);
    else if (matches_keyword(lex, "for")) set_keyword(lex, T_FOR);
    else if (matches_keyword(lex, "if")) set_keyword(lex, T_IF);
    else if (matches_keyword(lex, "in")) set_keyword(lex, T_IN);
    else if (matches_keyword(lex, "let")) set_keyword(lex, T_LET);
    else if (matches_keyword(lex, "print")) set_keyword(lex, T_PRINT);
    else if (matches_keyword(lex, "return")) set_keyword(lex, T_RET);
    else if (matches_keyword(lex, "undef")) set_keyword(lex, T_UNDEF);
    else if (matches_keyword(lex, "while")) set_keyword(lex, T_WHILE);
    // NOTE: special case for bools and floats
    else if (matches_keyword(lex, "nan") || matches_keyword(lex, "inf")) lex->type = T_FLOAT64;
    else if (matches_keyword(lex, "true") || matches_keyword(lex, "false")) lex->type = T_BOOL;
}

// Note: keep in sync with token.h
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
        "in",           // T_IN
        "id",           // T_ID,
        "let",          // T_LET,
        "const",        // T_CONST,
        "fn",           // T_FN,
        "return",       // T_RET,
        "enum",         // T_ENUM,
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
        "|",            // BAR,
        "|=",           // BAREQ,
        "||",           // DBAR,
        "||=",          // DBAREQ,
        "?",            // QMARK,
        "??",           // DQMARK,
        "?\?=",         // DQMARKEQ,
        ":",            // COLON,
        "::",           // DCOLON,
        "=>",           // BIG_ARR,
        "->"            // SMALL_ARR
};

Lexer *lex_new(FILE *file /* OWN */) {
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
    free(lex);
}
