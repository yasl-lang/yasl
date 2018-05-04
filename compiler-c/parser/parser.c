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

Token eattok(Parser *parser, Token token) {
    if (parser->lex->type != token) {
        printf("ParsingError: Expected %s, got %s\n", YASL_TOKEN_NAMES[token], YASL_TOKEN_NAMES[parser->lex->type]);
        exit(EXIT_FAILURE);
    }
    gettok(parser->lex);
    return token;
}

Node *parse(Parser *parser) {
    return parse_program(parser);
}

Node *parse_program(Parser *parser) {
    printf("parse. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    if (parser->lex->type == TOK_PRINT) {
        eattok(parser, TOK_PRINT);
        return new_Print(parse_expr(parser));
    } else if (parser->lex->type == TOK_LET) {
        return parse_let(parser);
    } else return new_ExprStmt(parse_expr(parser));
    //printf("ParsingError: Unknown sequence starting with %s\n", YASL_TOKEN_NAMES[parser->lex->type]);
    //puts("ParsingError: Unknown sequence.");
    exit(EXIT_FAILURE);
}

Node *parse_let(Parser *parser) {
    eattok(parser, TOK_LET);
    char *name = malloc(parser->lex->val_len);
    memcpy(name, parser->lex->value, parser->lex->val_len);
    int64_t name_len = parser->lex->val_len;
    eattok(parser, TOK_ID);
    if (parser->lex->type != TOK_EQ) return new_Let(name, name_len, NULL);
    eattok(parser, TOK_EQ);
    return new_Let(name, name_len, parse_expr(parser));
}

Node *parse_expr(Parser *parser) {
    return parse_assign(parser);
}

Node *parse_assign(Parser *parser) {
    Node *cur_node = parse_ternary(parser);
    if (parser->lex->type == TOK_EQ) { // || parser->lex->type == TOK_DLT)
        eattok(parser, TOK_EQ);
        if (cur_node->nodetype == NODE_VAR) {
            return new_Assign(cur_node->name, cur_node->name_len, parse_assign(parser));
        }
        // TODO: add indexing case
    }
    return cur_node;
}

Node *parse_ternary(Parser *parser) {
    Node *cur_node = parse_or(parser);
    if (parser->lex->type == TOK_DQMARK) {
        eattok(parser, TOK_DQMARK);
        return new_BinOp(TOK_DQMARK, cur_node, parse_ternary(parser));
    } else if (parser->lex->type == TOK_QMARK) {
        eattok(parser, TOK_QMARK);
        Node *left = parse_ternary(parser);
        eattok(parser, TOK_COLON);
        Node *right = parse_ternary(parser);
        return new_TriOp(TOK_QMARK, cur_node, left, right);
    }
    return cur_node;
}

Node *parse_or(Parser *parser) {
    Node *cur_node = parse_and(parser);
    while (parser->lex->type == TOK_OR) {
        eattok(parser, TOK_OR);
        cur_node = new_BinOp(TOK_OR, cur_node, parse_and(parser));
    }
    return cur_node;
}

Node *parse_and(Parser *parser) {
    Node *cur_node = parse_bor(parser);
    while (parser->lex->type == TOK_AND) {
        eattok(parser, TOK_AND);
        cur_node = new_BinOp(TOK_AND, cur_node, parse_bor(parser));
    }
    return cur_node;
}

Node *parse_bor(Parser *parser) {
    Node *cur_node = parse_bxor(parser);
    while (parser->lex->type == TOK_BAR) {
        eattok(parser, TOK_BAR);
        cur_node = new_BinOp(TOK_BAR, cur_node, parse_bxor(parser));
    }
    return cur_node;
}

Node *parse_bxor(Parser *parser) {
    //printf("bxor. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = parse_band(parser);
    while (parser->lex->type == TOK_TILDE) {
        eattok(parser, TOK_TILDE);
        cur_node = new_BinOp(TOK_TILDE, cur_node, parse_band(parser));
    }
    return cur_node;
}

Node *parse_band(Parser *parser) {
    //printf("band. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = parse_equals(parser);
    while (parser->lex->type == TOK_AMP) {
        eattok(parser, TOK_AMP);
        cur_node = new_BinOp(TOK_AMP, cur_node, parse_equals(parser));
    }
    return cur_node;
}

Node *parse_equals(Parser *parser) {
    //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = parse_comparator(parser);
    while (parser->lex->type == TOK_DEQ || parser->lex->type == TOK_BANGEQ ||
            parser->lex->type == TOK_TEQ || parser->lex->type == TOK_BANGDEQ) {
        Token op = eattok(parser, parser->lex->type);
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_comparator(parser));
    }
    return cur_node;
}

Node *parse_comparator(Parser *parser) {
    Node *cur_node = parse_concat(parser);
    while (parser->lex->type == TOK_LT || parser->lex->type == TOK_GT||
            parser->lex->type == TOK_GTEQ || parser->lex->type == TOK_LTEQ) {
        Token op = eattok(parser, parser->lex->type);
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
    if (parser->lex->type == TOK_DBAR || parser->lex->type == TOK_TBAR) {
        Token op = eattok(parser, parser->lex->type);
        return new_BinOp(op, cur_node, parse_concat(parser));
    }
    return cur_node;
}

Node *parse_bshift(Parser *parser) {
    Node *cur_node = parse_add(parser);
    while (parser->lex->type == TOK_DGT || parser->lex->type == TOK_DLT) {
        Token op = eattok(parser, parser->lex->type);
        cur_node = new_BinOp(op, cur_node, parse_add(parser));
    }
    return cur_node;
}

Node *parse_add(Parser *parser) {
    Node *cur_node = parse_multiply(parser);
    while (parser->lex->type == TOK_PLUS || parser->lex->type == TOK_MINUS) {
        Token op = eattok(parser, parser->lex->type);
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_multiply(parser));
    }
    return cur_node;
}

Node *parse_multiply(Parser *parser) {
    Node *cur_node = parse_unary(parser);
    while (parser->lex->type == TOK_STAR || parser->lex->type == TOK_SLASH ||
            parser->lex->type == TOK_DSLASH || parser->lex->type == TOK_MOD) {
        Token op = eattok(parser, parser->lex->type);
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
    //puts("made it here");
    if (parser->lex->type == TOK_PLUS || parser->lex->type == TOK_MINUS ||parser->lex->type == TOK_BANG ||
     parser->lex->type == TOK_TILDE ||parser->lex->type == TOK_HASH) {
        Token op = eattok(parser, parser->lex->type);
        return new_UnOp(op, parse_unary(parser));
    } else {
        return parse_power(parser);
    }
}

Node *parse_power(Parser *parser) {
    Node *cur_node = parse_constant(parser);
    if (parser->lex->type == TOK_CARET) { // || parser->lex->type == TOK_DLT)
        eattok(parser, TOK_CARET);
        return new_BinOp(TOK_CARET, cur_node, parse_power(parser));
    }
    return cur_node;
}

Node *parse_constant(Parser *parser) {
    //printf("comparator. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    if (parser->lex->type == TOK_ID) return parse_id(parser);
    else if (parser->lex->type == TOK_STR) return parse_string(parser);
    else if (parser->lex->type == TOK_INT64) return parse_integer(parser);
    else if (parser->lex->type == TOK_FLOAT64) return parse_float(parser);
    else if (parser->lex->type == TOK_BOOL) return parse_boolean(parser);
    else if (parser->lex->type == TOK_UNDEF) return parse_undef(parser);
    puts("ParsingError: Invalid exdpresion.");
    exit(EXIT_FAILURE);
}

Node *parse_id(Parser *parser) {
    char *name = malloc(parser->lex->val_len);
    memcpy(name, parser->lex->value, parser->lex->val_len);
    int64_t name_len = parser->lex->val_len;
    //printf("id: %s\n", parser->lex->value);
    eattok(parser, TOK_ID);
    if (parser->lex->type == TOK_LPAR) {
        puts("function call");
        // TODO: function calls
    } else if (parser->lex->type == TOK_LSQB) {
        puts("index");
        // TODO: indexing
    } else if (parser->lex->type == TOK_DOT) {
        puts("member access");
        // TODO: member access
    } else {
        puts("var");
        Node *cur_node = new_Var(name, name_len);
        //printf("id: %s\n", name);
        free(name);  // TODO: freeing this causes segfault.
        return cur_node;
    }
}

Node *parse_undef(Parser *parser) {
    Node *cur_node = new_Undef();
    eattok(parser, TOK_UNDEF);
    return cur_node;
}

Node *parse_float(Parser *parser) {
    Node* cur_node = new_Float(parser->lex->value, parser->lex->val_len);
    eattok(parser, TOK_FLOAT64);
    return cur_node;
}

Node *parse_integer(Parser *parser) {
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    Node *cur_node = new_Integer(parser->lex->value, parser->lex->val_len);
    eattok(parser, TOK_INT64);
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    return cur_node;
}

Node *parse_boolean(Parser *parser) {
    Node *cur_node = new_Boolean(parser->lex->value, parser->lex->val_len);
    eattok(parser, TOK_BOOL);
    return cur_node;
}

Node *parse_string(Parser *parser) {
    Node *cur_node = new_String(parser->lex->value, parser->lex->val_len);
    eattok(parser, TOK_STR);
    return cur_node;
}