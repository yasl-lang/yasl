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
    Node *cur_node = parse_concat(parser);
    while (parser->lex->type == TOK_LT || parser->lex->type == TOK_GT||
            parser->lex->type == TOK_GTEQ || parser->lex->type == TOK_LTEQ) {
        Token op = parser->lex->type;
        gettok(parser->lex);
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_concat(parser));
    }
    return cur_node;
}

Node *parse_concat(Parser *parser) {
    /*curr = self.bit_shift()
        if self.current_token.value in ("||", "|||"):
            token = self.eat(TokenTypes.OP)
            return BinOp(token, curr, self.concat())
        return curr */
    Node *cur_node = parse_bshift(parser);
    if (parser->lex->type == TOK_DBAR) { // || parser->lex->type == TOK_DLT)
        Token op = parser->lex->type;
        gettok(parser->lex);
        return new_BinOp(op, cur_node, parse_concat(parser));
    }
    return cur_node;
}

Node *parse_bshift(Parser *parser) {
    Node *cur_node = parse_add(parser);
    while (parser->lex->type == TOK_DGT || parser->lex->type == TOK_DLT) {
        Token op = parser->lex->type;
        gettok(parser->lex);
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_add(parser));
    }
    return cur_node;
}

Node *parse_add(Parser *parser) {
    Node *cur_node = parse_multiply(parser);
    while (parser->lex->type == TOK_PLUS || parser->lex->type == TOK_MINUS) {
        Token op = parser->lex->type;
        gettok(parser->lex);
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_multiply(parser));
    }
    return cur_node;
}

Node *parse_multiply(Parser *parser) {
    Node *cur_node = parse_unary(parser);
    while (parser->lex->type == TOK_STAR || parser->lex->type == TOK_SLASH ||
            parser->lex->type == TOK_DSLASH || parser->lex->type == TOK_MOD) {
        Token op = parser->lex->type;
        gettok(parser->lex);
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_unary(parser));
    }
    return cur_node;
}
/*
        if self.current_token.type is TokenTypes.OP and self.current_token.value in ("-", "+", "!", "#", "~"):
            op = self.eat(TokenTypes.OP)
            return UnOp(op, self.unop())
        else:
            return self.exponentiation()*/
Node *parse_unary(Parser *parser) {
    puts("made it here");
    if (parser->lex->type == TOK_PLUS || parser->lex->type == TOK_MINUS ||parser->lex->type == TOK_BANG ||
     parser->lex->type == TOK_TILDE ||parser->lex->type == TOK_HASH) {
        Token op = parser->lex->type;
        gettok(parser->lex);
        return new_UnOp(op, parse_unary(parser));
    } else {
        return parse_power(parser);
    }
}

Node *parse_power(Parser *parser) {
    Node *cur_node = parse_constant(parser);
    if (parser->lex->type == TOK_CARET) { // || parser->lex->type == TOK_DLT)
        Token op = parser->lex->type;
        gettok(parser->lex);
        return new_BinOp(op, cur_node, parse_power(parser));
    }
    return cur_node;
}

Node *parse_constant(Parser *parser) {
    //printf("comparator. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    if (parser->lex->type == TOK_STR) return parse_string(parser);
    else if (parser->lex->type == TOK_INT64) return parse_integer(parser);
    else if (parser->lex->type == TOK_FLOAT64) return parse_float(parser);
    puts("ParsingError: Invalid exdpresion.");
    exit(EXIT_FAILURE);
}

Node *parse_float(Parser *parser) {
    Node* cur_node = new_Float(parser->lex->value, parser->lex->val_len);
    gettok(parser->lex);
    return cur_node;
}

Node *parse_integer(Parser *parser) {
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node* cur_node = new_Integer(parser->lex->value, parser->lex->val_len);
    gettok(parser->lex);
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    return cur_node;
}

Node *parse_string(Parser *parser) {
    //gettok(parser->lex);
    Node* cur_node = new_String(parser->lex->value, parser->lex->val_len);
    gettok(parser->lex);
    return cur_node;
}