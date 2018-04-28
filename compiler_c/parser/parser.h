#pragma once

#include "../lexer/lexer.h"

typedef struct {
    Lexer *lex;
} Parser;


Parser *parser_new(Lexer *lex);
void parser_del(Parser *parser);