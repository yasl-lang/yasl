#include <lexer.h>
#include <compiler/lexer/lexer.h>
#include <color.h>

#define ASSERT_EQ(left, right) do {\
            if ((left) == (right)) {\
                printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);\
            } else {\
                printf(K_RED "assert failed in %s: line %d" K_END "\n", __func__, __LINE__);\
            }\
        } while(0)

#define ASSERT_TOK_EQ(left, right) do {\
    if (left == right) {\
        printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);\
    } else {\
        printf(K_RED "assert failed in %s: line %d: %s =/= %s" K_END "\n", __func__, __LINE__, YASL_TOKEN_NAMES[left], YASL_TOKEN_NAMES[right]);\
    }\
} while(0)

#define ASSERT_EATTOK(tok, lex) do {\
            gettok(lex);\
            ASSERT_TOK_EQ(tok, (lex)->type);\
        } while(0)

Lexer *setup_lexer(char *file_contents) {
    FILE *fptr = fopen("dump.ysl", "w");
    fwrite(file_contents, 1, strlen(file_contents), fptr);
    fseek(fptr, 0, SEEK_SET);
    fclose(fptr);
    fptr = fopen("dump.ysl", "r");
    return lex_new(fptr);
}

void test_int(void) {
    Lexer *lex = setup_lexer("64\n");
    ASSERT_EATTOK(T_INT64, lex);
    ASSERT_EATTOK(T_SEMI, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_string(void) {
    Lexer *lex = setup_lexer("let x = 'hello world';");
    ASSERT_EATTOK(T_LET, lex);
    ASSERT_EATTOK(T_ID, lex);
    ASSERT_EATTOK(T_EQ, lex);
    ASSERT_EATTOK(T_STR, lex);
    ASSERT_EATTOK(T_SEMI, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_division(void) {
    Lexer *lex = setup_lexer("5 / 7.0");
    ASSERT_EATTOK(T_INT64, lex);
    ASSERT_EATTOK(T_SLASH, lex);
    ASSERT_EATTOK(T_FLOAT64, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_blockcomment(void) {
    Lexer *lex = setup_lexer("/* block comment */ while x > 0 { /* print x */ print x\nx -= 1\n}");
    ASSERT_EATTOK(T_WHILE, lex);
    ASSERT_EATTOK(T_ID, lex);
    ASSERT_EATTOK(T_GT, lex);
    ASSERT_EATTOK(T_INT64, lex);
    ASSERT_EATTOK(T_LBRC, lex);
    ASSERT_EATTOK(T_PRINT, lex);
    ASSERT_EATTOK(T_ID, lex);
    ASSERT_EATTOK(T_SEMI, lex);
    ASSERT_EATTOK(T_ID, lex);
    ASSERT_EATTOK(T_MINUSEQ, lex);
    ASSERT_EATTOK(T_INT64, lex);
    ASSERT_EATTOK(T_SEMI, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

int main() {
    test_int();
    test_string();
    test_division();
}