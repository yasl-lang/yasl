#include "lexer.h"
#include "../token.h"

void gettok(Lexer *lex) {
    //puts("getting next");
    //printf("%ld chars from the start.\n", ftell(lex->file));
    char c1, c2, c3, c4;
    int last;
    //printf("last char is %c\n", lastchar);
    c1 = fgetc(lex->file);

    while (!feof(lex->file) && (c1 == ' ' || c1 == '\n' || c1 == '\t')) {
        if (c1 == '\n') lex->line++;
        c1 = fgetc(lex->file);
    }

    if (feof(lex->file)) {
        lex->type = TOK_EOF;
        lex->value = realloc(lex->value, 0);
        return;
    }

    if (c1 == '$') {                            // comments
        c1 = fgetc(lex->file);
        if (c1 == '$') {
            while (!feof(lex->file) && fgetc(lex->file) != '\n') {}
        } else if (c1 == '*') {
            c1 = fgetc(lex->file);
            c2 = fgetc(lex->file);
            while (!feof(lex->file) && (c1 != '*' || c2 != '$')) {
                c1 = c2;
                c2 = fgetc(lex->file);
            }
            if (feof(lex->file)) {
                puts("LexingError: unclosed block comment.");
                exit(EXIT_FAILURE);
            }
        } else {
            fseek(lex->file, -2, SEEK_CUR);
        }
        c1 = fgetc(lex->file);
    }

    while (!feof(lex->file) && (c1 == ' ' || c1 == '\n' || c1 == '\t')) {
        if (c1 == '\n') lex->line++;
        c1 = fgetc(lex->file);
    }

    if (feof(lex->file)) {
        lex->type = TOK_EOF;
        lex->value = NULL;
        return;
    }

    if (isdigit(c1)) {                          // numbers
        lex->val_len = 6;
        lex->value = realloc(lex->value, lex->val_len);
        int i = 0;
        c2 = fgetc(lex->file);
        if (c1 == '0' && c2 == 'x'){            // hexadecimal literal
            lex->value[i++] = '0';
            lex->value[i++] = 'x';
            c1 = fgetc(lex->file);
            do {
                lex->value[i++] = c1;
                c1 = fgetc(lex->file);
                if (i == lex->val_len) {
                    lex->val_len *= 2;
                    lex->value = realloc(lex->value, lex->val_len);
                }
            } while (!feof(lex->file) && isxdigit(c1));    // isxdigit checks if a hex digit.
            lex->type = TOK_INT64;
            if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
            return;
        } else if (c1 == '0' && c2 == 'b'){         // binary literal
            lex->value[i++] = '0';
            lex->value[i++] = 'b';
            c1 = fgetc(lex->file);
            do {
                lex->value[i++] = c1;
                c1 = fgetc(lex->file);
                if (i == lex->val_len) {
                    lex->val_len *= 2;
                    lex->value = realloc(lex->value, lex->val_len);
                }
            } while (!feof(lex->file) && isbdigit(c1));    // isbdigit checks if a binary digit ('1' or '0').
            lex->type = TOK_INT64;
            if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
            return;
        } else {
            fseek(lex->file, -1, SEEK_CUR);
        }

        do {
            lex->value[i++] = c1;
            c1 = fgetc(lex->file);
            if (i == lex->val_len) {
                lex->val_len *= 2;
                lex->value = realloc(lex->value, lex->val_len);
            }
        } while (!feof(lex->file) && isdigit(c1));
        lex->type = TOK_INT64;

        if (c1 == '.') {                    // floats
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
                c1 = fgetc(lex->file);
                if (i == lex->val_len) {
                    lex->val_len *= 2;
                    lex->value = realloc(lex->value, lex->val_len);
                }
            } while (!feof(lex->file) && isdigit(c1));

            if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
            lex->type = TOK_FLOAT64;
            return;
        }

        if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
        return;
    } else if (isalpha(c1)) {                           // identifiers and keywords
        lex->val_len = 6;
        lex->value = realloc(lex->value, lex->val_len);
        int i = 0;
        do {
            lex->value[i++] = c1;
            c1 = fgetc(lex->file);
            if (i == lex->val_len) {
                lex->val_len *= 2;
                lex->value = realloc(lex->value, lex->val_len);
            }
        } while (!feof(lex->file) && isalnum(c1));
        if (!feof(lex->file)) fseek(lex->file, -1, SEEK_CUR);
        lex->value = realloc(lex->value, lex->val_len = i);
        lex->type = TOK_ID;
        YASLKeywords(lex);          // keywords
        return;
    } else if (c1 == '"') {                             // strings
        lex->val_len = 6;
        lex->value = realloc(lex->value, lex->val_len);
        int i = 0;
        c1 = fgetc(lex->file);
        while (c1 != '"' && !feof(lex->file)) {
            lex->value[i++] = c1;
            c1 = fgetc(lex->file);
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
        lex->type = TOK_STR;
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
    //puts("four");
    last = YASLToken_FourChars(c1, c2, c3, c4);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }
    fseek(lex->file, -1, SEEK_CUR);

    three:
    //puts("three");
    last = YASLToken_ThreeChars(c1, c2, c3);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }
    fseek(lex->file, -1, SEEK_CUR);

    two:
    //puts("two");
    last = YASLToken_TwoChars(c1, c2);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }
    fseek(lex->file, -1, SEEK_CUR);

    one:
    //puts("one");
    last = YASLToken_OneChar(c1);
    if (last != -1) {
        lex->type = last;
        lex->value = realloc(lex->value, 0);
        return;
    }

    puts("LexingError: unknown lexeme.");
    exit(EXIT_FAILURE);
}

Token YASLToken_FourChars(char c1, char c2, char c3, char c4) {
    switch(c1) { case '|': switch(c2) { case '|': switch(c3) { case '|':  switch(c4) { case '=': return TOK_TBAREQ; default: return UNKNOWN;} } } }
    return UNKNOWN;
}

Token YASLToken_ThreeChars(char c1, char c2, char c3) {
    switch(c1) {
        case '<': switch(c2) { case '<': switch(c3) { case '=': return TOK_DLTEQ; default: return UNKNOWN;} }
        case '>': switch(c2) { case '>': switch(c3) { case '=': return TOK_DGTEQ; default: return UNKNOWN;} }
        case '=': switch(c2) { case '=': switch(c3) { case '=': return TOK_TEQ; default: return UNKNOWN;} }
        case '!': switch(c2) { case '=': switch(c3) { case '=': return TOK_BANGDEQ; default: return UNKNOWN;} }
        case '/': switch(c2) { case '/': switch(c3) { case '=': return TOK_DSLASHEQ; default: return UNKNOWN;} }
        case '|': switch(c2) { case '|': switch(c3) {
                        case '=': return TOK_DBAREQ;
                        case '|': return TOK_TBAR;
                        default: return UNKNOWN;
        } }
        case '?': switch(c2) { case '?': switch(c3) { case '=': return TOK_DQMARKEQ; default: return UNKNOWN;} }
    }
    return UNKNOWN;
}

Token YASLToken_TwoChars(char c1, char c2) {
    switch(c1) {
        case '^': switch(c2) { case '=': return TOK_CARETEQ; default: return UNKNOWN; };
        case '+': switch(c2) { case '=': return TOK_PLUSEQ; default: return UNKNOWN; };
        case '-': switch(c2) {
                case '=': return TOK_MINUSEQ;
                case '>': return TOK_RARR;
                default: return UNKNOWN;
        }
        case '=': switch(c2) { case '=': return TOK_DEQ; default: return UNKNOWN;}
        case '!': switch(c2) { case '=': return TOK_BANGEQ; default: return UNKNOWN;}
        case '~': switch(c2) { case '=': return TOK_TILDEEQ; default: return UNKNOWN;}
        case '*': switch(c2) { case '=': return TOK_STAREQ; default: return UNKNOWN;}
        case '/': switch(c2) { case '=': return TOK_SLASHEQ; default: return UNKNOWN; }
        case '%': switch(c2) { case '=': return TOK_MODEQ; default: return UNKNOWN;}
        case '<': switch(c2) {
                case '=': return TOK_LTEQ;
                case '<': return TOK_DLT;
                case '-': return TOK_LARR;
                default: return UNKNOWN;
        }
        case '>': switch(c2) {
                case '=': return TOK_GTEQ;
                case '>': return TOK_DGT;
                default:  return UNKNOWN;
        }
        case '&': switch(c2) { case '=': return TOK_AMPEQ; default: return UNKNOWN; }
        case '|': switch(c2) {
                case '=': return TOK_BAREQ;
                case '|': return TOK_DBAR;
                default: return UNKNOWN;
        }
        case '?': switch(c2) { case '?': return TOK_DQMARK; default: return UNKNOWN;}
    }
    return UNKNOWN;
}

Token YASLToken_OneChar(char c1) {
    switch(c1) {
        case '(': return TOK_LPAR;
        case ')': return TOK_RPAR;
        case '[': return TOK_LSQB;
        case ']': return TOK_RSQB;
        case '{': return TOK_LBRC;
        case '}': return TOK_RBRC;
        case '.': return TOK_DOT;
        case ',': return TOK_COMMA;
        case '^': return TOK_CARET;
        case '+': return TOK_PLUS;
        case '-': return TOK_MINUS;
        case '#': return TOK_HASH;
        case '!': return TOK_BANG;
        case '~': return TOK_TILDE;
        case '*': return TOK_STAR;
        case '/': return TOK_SLASH;
        case '%': return TOK_MOD;
        case '<': return TOK_LT;
        case '>': return TOK_GT;
        case '=': return TOK_EQ;
        case '&': return TOK_AMP;
        case '|': return TOK_BAR;
        case '?': return TOK_QMARK;
        case ':': return TOK_COLON;
        default: return UNKNOWN;
    }
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
    if (!memcmp(lex->value, "let", lex->val_len)) {
        lex->type = TOK_LET;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "print", lex->val_len)) {
        lex->type = TOK_PRINT;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "else", lex->val_len)) {
        lex->type = TOK_ELSE;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "if", lex->val_len)) {
        lex->type = TOK_IF;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "elseif", lex->val_len)) {
        lex->type = TOK_ELSEIF;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "while", lex->val_len)) {
        lex->type = TOK_WHILE;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "for", lex->val_len)) {
        lex->type = TOK_FOR;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "break", lex->val_len)) {
        lex->type = TOK_BREAK;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "continue", lex->val_len)) {
        lex->type = TOK_CONT;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "true", lex->val_len)) {
        lex->type = TOK_BOOL;
    } else if (!memcmp(lex->value, "false", lex->val_len)) {
        lex->type = TOK_BOOL;
    } else if (!memcmp(lex->value, "or", lex->val_len)) {
        lex->type = TOK_OR;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "and", lex->val_len)) {
        lex->type = TOK_AND;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "undef", lex->val_len)) {
        lex->type = TOK_UNDEF;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "fn", lex->val_len)) {
        lex->type = TOK_FN;
        lex->value = realloc(lex->value, 0);
    } else if (!memcmp(lex->value, "return", lex->val_len)) {
        lex->type = TOK_RET;
        lex->value = realloc(lex->value, 0);
    }
}

const char *YASL_TOKEN_NAMES[] = {
        "END OF FILE",  // TOK_EOF,
        ";",            // TOK_SEMI,
        "undef",        // TOK_UNDEF,
        "float64",      // TOK_FLOAT64,
        "int64",        // TOK_INT64,
        "bool",         // TOK_BOOL,
        "str",          // TOK_STR,
        "if",           // TOK_IF,
        "elseif",       // TOK_ELSEIF,
        "else",         // TOK_ELSE,
        "while",        // TOK_WHILE,
        "break",        // TOK_BREAK,
        "continue",     // TOK_CONT,
        "for",          // TOK_FOR,
        "and",          // TOK_AND,
        "or",           // TOK_OR,
        "id",           // TOK_ID,
        "let",          // TOK_LET,
        "fn",           // TOK_FN,
        "return",       // TOK_RET,
        "struct",       // TOK_STRUCT,
        "print",        // TOK_PRINT,
        "(",            // LPAR,
        ")",            // RPAR,
        "[",            // LSQB,
        "]",            // RSQB,
        "{",            // LBRC,
        "}",            // RBRC,
        ".",            // DOT,
        ",",            // COMMA,
        "^",            // CARET,
        "+",            // PLUS,
        "-",            // MINUS,
        "#",            // HASH,
        "!",            // BANG,
        "~",            // TILDE,
        "*",            // STAR,
        "/",            // SLASH,
        "%",            // MOD,
        "<",            // LT,
        ">",            // GT,
        "=",            // EQ,
        "&",            // AMP,
        "|",            // BAR,
        "?",            // QMARK,
        ":",            // COLON,
        "^=",           //CARETEQ,
        "+=",           // PLUSEQ,
        "-=",           // MINUSEQ,
        "!=",           // BANGEQ,
        "==",           // DEQ,
        "~=",           // TILDEEQ,
        "*=",           // STAREQ,
        "/=",           // SLASHEQ,
        "%=",           // MODEQ,
        "<<",           // DLT,
        ">>",           // DGT,
        "<=",           // LTEQ,
        ">=",           // GTEQ,
        "&=",           // AMPEQ,
        "|=",           // BAREQ,
        "||",           // DBAR,
        "??",           // DQMARK,
        "->",           // RARR,
        "<-",           // LARR,
        "<<=",          // DLTEQ,
        ">>=",          // DGTEQ,
        "===",          // TEQ,
        "!==",          // BANGDEQ,
        "//=",          // DSLASHEQ,
        "||=",          // DBAREQ,
        "|||",          // TBAR,
        "?\?=",          // DQMARKEQ,
        "|||=",         // TBAREQ,
};

Lexer *lex_new(FILE *file) {
    Lexer *lex = malloc(sizeof(Lexer));
    lex->line = -1;
    lex->value = NULL;
    lex->val_len = 0;
    lex->file = file;
    lex->type = UNKNOWN;
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
    while (lex->type != TOK_EOF) {
        gettok(lex);
        printf("type: %s, value: %s\n", YASL_TOKEN_NAMES[lex->type], lex->value);
    }
    lex_del(lex);
}
*/