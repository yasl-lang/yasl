#include "parser.h"

Parser *parser_new(Lexer *lex) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lex = lex;
    return parser;
};

void parser_del(Parser *parser) {
    lex_del(parser->lex);
    free(parser);
};

Node *parse(Parser *parser) {
    return parse_program(parser);
}

Node *parse_program(Parser *parser) {
    //printf("parse_program. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    if (parser->lex->type == TOK_PRINT) {
        gettok(parser->lex);
        return new_Print(parse_expr(parser));
    }
    puts("ParsingError: Unknown sequence.");
    exit(EXIT_FAILURE);
}

Node *parse_expr(Parser *parser) {
    return parse_or(parser);
}

Node *parse_or(Parser *parser) {
    return parse_and(parser);
}

Node *parse_and(Parser *parser) {
    return parse_bor(parser);
}

/*
curr = self.logic_and()
while self.current_token.value == "or":
    op = self.eat(TokenTypes.LOGIC)
    curr = LogicOp(op, curr, self.logic_and())
return curr
*/

Node *parse_bor(Parser *parser) {
    //printf("bor. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = parse_bxor(parser);
    while (parser->lex->type == TOK_BAR) {
        gettok(parser->lex);
        cur_node = new_BinOp(TOK_BAR, cur_node, parse_bxor(parser));
    }
    return cur_node;
}

Node *parse_bxor(Parser *parser) {
    //printf("bxor. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = parse_band(parser);
    while (parser->lex->type == TOK_TILDE) {
        gettok(parser->lex);
        cur_node = new_BinOp(TOK_TILDE, cur_node, parse_band(parser));
    }
    return cur_node;
}

Node *parse_band(Parser *parser) {
    //printf("band. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = parse_equals(parser);
    while (parser->lex->type == TOK_AMP) {
        gettok(parser->lex);
        cur_node = new_BinOp(TOK_AMP, cur_node, parse_equals(parser));
    }
    return cur_node;
}

Node *parse_equals(Parser *parser) {
    //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = parse_comparator(parser);
    while (parser->lex->type == TOK_DEQ || parser->lex->type == TOK_BANGEQ ||
            parser->lex->type == TOK_TEQ || parser->lex->type == TOK_BANGDEQ) {
        Token op = parser->lex->type;
        gettok(parser->lex);
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_comparator(parser));
    }
    return cur_node;
}

Node *parse_comparator(Parser *parser) {
    //printf("comparator. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    if (parser->lex->type == TOK_STR) return parse_string(parser);
    else if (parser->lex->type == TOK_INT64) return parse_integer(parser);
    puts("ParsingError: Invalid exdpresion.");
    exit(EXIT_FAILURE);
}

Node *parse_concat(Parser *parser);
Node *parse_add(Parser *parser);
Node *parse_multiply(Parser *parser);
Node *parse_unary(Parser *parser);
Node *parse_power(Parser *parser);

Node *parse_integer(Parser *parser) {
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node* cur_node = new_Integer(parser->lex->value, parser->lex->val_len);
    gettok(parser->lex);
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    return cur_node;
}

Node *parse_string(Parser *parser) {
    //gettok(parser->lex);
    return new_String(parser->lex->value, parser->lex->val_len);
}