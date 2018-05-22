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
    if (curtok(parser) != token) {
        printf("ParsingError: Expected %s, got %s\n", YASL_TOKEN_NAMES[token], YASL_TOKEN_NAMES[curtok(parser)]);
        exit(EXIT_FAILURE);
    }
    gettok(parser->lex);
    return token;
}

Node *parse(Parser *parser) {
    return parse_program(parser);
}

Node *parse_program(Parser *parser) {
    printf("parse. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
    if (curtok(parser) == TOK_PRINT) {
        eattok(parser, TOK_PRINT);
        puts("about to do new Print");
        return new_Print(parse_expr(parser), parser->lex->line);
    } else if (curtok(parser) == TOK_LET) {
        return parse_let(parser);
    } else if (curtok(parser) == TOK_WHILE) {
        return parse_while(parser);
    } else if (curtok(parser) == TOK_BREAK) {
        eattok(parser, TOK_BREAK);
        return new_Break(parser->lex->line);
    } else if (curtok(parser) == TOK_CONT) {
        eattok(parser, TOK_CONT);
        return new_Continue(parser->lex->line);
    } else if (curtok(parser) == TOK_IF) {
        return parse_if(parser);
    } else if (curtok(parser) == TOK_ELSEIF) {
        puts("ParsingError: elseif without previous if");
        exit(EXIT_FAILURE);
    } else if (curtok(parser) == TOK_ELSE) {
        puts("ParsingError: else without previous if");
        exit(EXIT_FAILURE);
    } else return new_ExprStmt(parse_expr(parser), parser->lex->line);
    //printf("ParsingError: Unknown sequence starting with %s\n", YASL_TOKEN_NAMES[curtok(parser)]);
    //puts("ParsingError: Unknown sequence.");
    exit(EXIT_FAILURE);
}

Node *parse_let(Parser *parser) {
    eattok(parser, TOK_LET);
    char *name = malloc(parser->lex->val_len);
    memcpy(name, parser->lex->value, parser->lex->val_len);
    int64_t name_len = parser->lex->val_len;
    eattok(parser, TOK_ID);
    if (curtok(parser) != TOK_EQ) return new_Let(name, name_len, NULL, parser->lex->line);
    eattok(parser, TOK_EQ);
    return new_Let(name, name_len, parse_expr(parser), parser->lex->line);
}

Node *parse_while(Parser *parser) {
    /*
     * token = self.eat(TokenTypes.WHILE)
     * cond = self.expr()
     * self.eat(TokenTypes.LBRACE)
     * body = []
     * while self.current_token.type is not TokenTypes.RBRACE and self.current_token.type is not TokenTypes.EOF:
     *     body.append(self.program())
     *     self.eat(TokenTypes.SEMI)
     * self.eat(TokenTypes.RBRACE)
     * return While(token, cond, body)
     */
    eattok(parser, TOK_WHILE);
    Node *cond = parse_expr(parser);
    eattok(parser, TOK_LBRC);
    Node *body = new_Block(parser->lex->line);
    while (curtok(parser) != TOK_RBRC && curtok(parser) != TOK_EOF) {
        block_append(body, parse_program(parser));
        if (curtok(parser) == TOK_SEMI) eattok(parser, TOK_SEMI);
        else if (curtok(parser) != TOK_RBRC) {
            puts("expected semicolon/newline or right brace");
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, TOK_RBRC);
    return new_While(cond, body, parser->lex->line);
}

Node *parse_if(Parser *parser) {
    /*
        if self.current_token.type is TokenTypes.IF:
            token = self.eat(TokenTypes.IF)
        else:
            token = self.eat(TokenTypes.ELSEIF)
        cond = self.expr()
        self.eat(TokenTypes.LBRACE)
        body = []
        while self.current_token.type is not TokenTypes.RBRACE:
            body.append(self.program())
            self.eat(TokenTypes.SEMI)
        self.eat(TokenTypes.RBRACE)
        if self.current_token.type is not TokenTypes.ELSE and self.current_token.type is not TokenTypes.ELSEIF:
            return If(token, cond, Block(body))
        if self.current_token.type is TokenTypes.SEMI:
            self.eat(TokenTypes.SEMI)
        if self.current_token.type is TokenTypes.ELSEIF:
            return IfElse(token, cond, Block(body), self.if_stmt())
        if self.current_token.type is TokenTypes.ELSE:
            left = body
            right = []
            self.eat(TokenTypes.ELSE)
            self.eat(TokenTypes.LBRACE)
            while self.current_token.type is not TokenTypes.RBRACE:
                right.append(self.program())
                self.eat(TokenTypes.SEMI)
            self.eat(TokenTypes.RBRACE)
            return IfElse(token, cond, Block(left), Block(right))
        assert False
     */
    if (curtok(parser) == TOK_IF) eattok(parser, TOK_IF);
    else if (curtok(parser) == TOK_ELSEIF) eattok(parser, TOK_ELSEIF);
    else {
        printf("ParsingError: Expected if or elseif, got %s\n", YASL_TOKEN_NAMES[curtok(parser)]);
        exit(EXIT_FAILURE);
    }
    Node *cond = parse_expr(parser);
    eattok(parser, TOK_LBRC);
    Node *then_block = new_Block(parser->lex->line);
    while (curtok(parser) != TOK_RBRC && curtok(parser) != TOK_EOF) {
        block_append(then_block, parse_program(parser));
        if (curtok(parser) == TOK_SEMI) eattok(parser, TOK_SEMI);
        else if (curtok(parser) != TOK_RBRC) {
            puts("expected semicolon/newline or right brace");
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, TOK_RBRC);
    if (curtok(parser) != TOK_ELSE && curtok(parser) != TOK_ELSEIF) {
        puts("No Else");
        return new_If(cond, then_block, NULL, parser->lex->line);
    }
    // TODO: eat semi
    if (curtok(parser) == TOK_ELSEIF) {
        puts("ElseIf");
        return new_If(cond, then_block, parse_if(parser), parser->lex->line);
    }
    if (curtok(parser) == TOK_ELSE) {
        puts("Else");
        eattok(parser, TOK_ELSE);
        eattok(parser, TOK_LBRC);
        Node *else_block = new_Block(parser->lex->line);
        while (curtok(parser) != TOK_RBRC && curtok(parser) != TOK_EOF) {
            block_append(else_block, parse_program(parser));
            if (curtok(parser) == TOK_SEMI) eattok(parser, TOK_SEMI);
            else if (curtok(parser) != TOK_RBRC) {
                puts("expected semicolon/newline or right brace");
                exit(EXIT_FAILURE);
            }
        }
        eattok(parser, TOK_RBRC);
        return new_If(cond, then_block, else_block, parser->lex->line);
    }
    printf("ParsingError: expected newline or semicolon, got %s\n", YASL_TOKEN_NAMES[curtok(parser)]);
    exit(EXIT_FAILURE);

}

Node *parse_expr(Parser *parser) {
    puts("parse expr");
    return parse_assign(parser);
}

Node *parse_assign(Parser *parser) {
    Node *cur_node = parse_ternary(parser);
    if (curtok(parser) == TOK_EQ) { // || curtok(parser) == TOK_DLT)
        eattok(parser, TOK_EQ);
        if (cur_node->nodetype == NODE_VAR) {
            Node *assign_node = new_Assign(cur_node->name, cur_node->name_len, parse_assign(parser), parser->lex->line);
            node_del(cur_node);
            return assign_node;
        }
        // TODO: add indexing case
    } else if (isaugmented(curtok(parser))) {
        Token op = eattok(parser, curtok(parser)) - 1; // relies on enum
        if (cur_node->nodetype == NODE_VAR) {
            return new_Assign(cur_node->name, cur_node->name_len, new_BinOp(op, cur_node, parse_assign(parser), parser->lex->line), parser->lex->line);
        }
        // TODO: add indexing case
    }
    return cur_node;
}

Node *parse_ternary(Parser *parser) {
    Node *cur_node = parse_or(parser);
    if (curtok(parser) == TOK_DQMARK) {
        eattok(parser, TOK_DQMARK);
        return new_BinOp(TOK_DQMARK, cur_node, parse_ternary(parser), parser->lex->line);
    } else if (curtok(parser) == TOK_QMARK) {
        eattok(parser, TOK_QMARK);
        Node *left = parse_ternary(parser);
        eattok(parser, TOK_COLON);
        Node *right = parse_ternary(parser);
        return new_TriOp(TOK_QMARK, cur_node, left, right, parser->lex->line);
    }
    return cur_node;
}

Node *parse_or(Parser *parser) {
    Node *cur_node = parse_and(parser);
    while (curtok(parser) == TOK_OR) {
        eattok(parser, TOK_OR);
        cur_node = new_BinOp(TOK_OR, cur_node, parse_and(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_and(Parser *parser) {
    Node *cur_node = parse_bor(parser);
    while (curtok(parser) == TOK_AND) {
        eattok(parser, TOK_AND);
        cur_node = new_BinOp(TOK_AND, cur_node, parse_bor(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_bor(Parser *parser) {
    Node *cur_node = parse_bxor(parser);
    while (curtok(parser) == TOK_BAR) {
        eattok(parser, TOK_BAR);
        cur_node = new_BinOp(TOK_BAR, cur_node, parse_bxor(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_bxor(Parser *parser) {
    //printf("bxor. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
    Node *cur_node = parse_band(parser);
    while (curtok(parser) == TOK_CARET) {
        eattok(parser, TOK_CARET);
        cur_node = new_BinOp(TOK_CARET, cur_node, parse_band(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_band(Parser *parser) {
    //printf("band. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
    Node *cur_node = parse_equals(parser);
    while (curtok(parser) == TOK_AMP) {
        eattok(parser, TOK_AMP);
        cur_node = new_BinOp(TOK_AMP, cur_node, parse_equals(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_equals(Parser *parser) {
    //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
    Node *cur_node = parse_comparator(parser);
    while (curtok(parser) == TOK_DEQ || curtok(parser) == TOK_BANGEQ ||
            curtok(parser) == TOK_TEQ || curtok(parser) == TOK_BANGDEQ) {
        Token op = eattok(parser, curtok(parser));
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_comparator(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_comparator(Parser *parser) {
    Node *cur_node = parse_concat(parser);
    while (curtok(parser) == TOK_LT || curtok(parser) == TOK_GT||
            curtok(parser) == TOK_GTEQ || curtok(parser) == TOK_LTEQ) {
        Token op = eattok(parser, curtok(parser));
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_concat(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_concat(Parser *parser) {
    Node *cur_node = parse_bshift(parser);
    if (curtok(parser) == TOK_DBAR || curtok(parser) == TOK_TBAR) {
        Token op = eattok(parser, curtok(parser));
        return new_BinOp(op, cur_node, parse_concat(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_bshift(Parser *parser) {
    Node *cur_node = parse_add(parser);
    while (curtok(parser) == TOK_DGT || curtok(parser) == TOK_DLT) {
        Token op = eattok(parser, curtok(parser));
        cur_node = new_BinOp(op, cur_node, parse_add(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_add(Parser *parser) {
    Node *cur_node = parse_multiply(parser);
    while (curtok(parser) == TOK_PLUS || curtok(parser) == TOK_MINUS) {
        Token op = eattok(parser, curtok(parser));
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_multiply(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_multiply(Parser *parser) {
    Node *cur_node = parse_unary(parser);
    while (curtok(parser) == TOK_STAR || curtok(parser) == TOK_SLASH ||
            curtok(parser) == TOK_DSLASH || curtok(parser) == TOK_MOD) {
        Token op = eattok(parser, curtok(parser));
        //printf("equals. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
        cur_node = new_BinOp(op, cur_node, parse_unary(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_unary(Parser *parser) {
    //puts("made it here");
    if (curtok(parser) == TOK_PLUS || curtok(parser) == TOK_MINUS || curtok(parser) == TOK_BANG ||
     curtok(parser) == TOK_CARET ||curtok(parser) == TOK_HASH) {
        Token op = eattok(parser, curtok(parser));
        return new_UnOp(op, parse_unary(parser), parser->lex->line);
    } else {
        return parse_power(parser);
    }
}

Node *parse_power(Parser *parser) {
    Node *cur_node = parse_call(parser);
    if (curtok(parser) == TOK_DSTAR) { // || curtok(parser) == TOK_DLT)
        eattok(parser, TOK_DSTAR);
        return new_BinOp(TOK_DSTAR, cur_node, parse_power(parser), parser->lex->line);
    }
    return cur_node;
}

Node *parse_call(Parser *parser) {
    /*
     *  result = self.literal()
        while self.current_token.type is TokenTypes.DOT or self.current_token.type is TokenTypes.LBRACK:
            if self.current_token.type is TokenTypes.DOT:
                self.eat(TokenTypes.DOT)
                right = self.literal()
                if isinstance(right, FunctionCall):
                    result = MethodCall(result, right.token, right.params)
                else:
                    assert False
            else:
                self.eat(TokenTypes.LBRACK)
                result = Index(result, self.expr())
                self.eat(TokenTypes.RBRACK)
        return result
     */
    Node *cur_node = parse_constant(parser);
    while (curtok(parser) == TOK_LSQB || curtok(parser) == TOK_DOT) {
        eattok(parser, TOK_LSQB);
        cur_node = new_Index(cur_node, parse_expr(parser), parser->lex->line);
        eattok(parser, TOK_RSQB);
    }
    /*while (curtok(parser) == TOK_DOT || curtok(parser) == TOK_LSQB) {
        /*if (curtok(parser) == TOK_DOT) {
            eattok(parser, TOK_DOT);
            Node *right = parse_constant(parser);
            if (right->nodetype == NODE_FN) {
                cur_node = new_MethodCall(...);
            } else if (right->nodetype == NODE_VAR) {
                cur_node = new_Member(...);
            }
        }
        eattok(parser, TOK_LSQB);
        // TODO: order of evaluation is undefined here
        cur_node = new_Index(cur_node, parse_expr(parser), parser->lex->line);
        eattok(parser, TOK_RSQB);
    }*/
    return cur_node;

}

Node *parse_constant(Parser *parser) {
    //printf("comparator. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
    if (curtok(parser) == TOK_ID) return parse_id(parser);
    else if (curtok(parser) == TOK_LSQB) return parse_collection(parser);
    else if (curtok(parser) == TOK_STR) return parse_string(parser);
    else if (curtok(parser) == TOK_INT64) return parse_integer(parser);
    else if (curtok(parser) == TOK_FLOAT64) return parse_float(parser);
    else if (curtok(parser) == TOK_BOOL) return parse_boolean(parser);
    else if (curtok(parser) == TOK_UNDEF) return parse_undef(parser);
    puts("ParsingError: Invalid expression.");
    exit(EXIT_FAILURE);
}

Node *parse_id(Parser *parser) {
    char *name = malloc(parser->lex->val_len);
    memcpy(name, parser->lex->value, parser->lex->val_len);
    int64_t name_len = parser->lex->val_len;
    //printf("id: %s\n", parser->lex->value);
    eattok(parser, TOK_ID);
    if (curtok(parser) == TOK_LPAR) {
        puts("function call");
        // TODO: function calls
    } /*else if (curtok(parser) == TOK_LSQB) {
        puts("index");
        // TODO: indexing
    } else if (curtok(parser) == TOK_DOT) {
        puts("member access");
        // TODO: member access
    } */ else {
        puts("var");
        Node *cur_node = new_Var(name, name_len, parser->lex->line);
        //printf("id: %s\n", name);
        free(name);
        return cur_node;
    }
}

Node *parse_undef(Parser *parser) {
    Node *cur_node = new_Undef(parser->lex->line);
    eattok(parser, TOK_UNDEF);
    return cur_node;
}

Node *parse_float(Parser *parser) {
    Node* cur_node = new_Float(parser->lex->value, parser->lex->val_len, parser->lex->line);
    eattok(parser, TOK_FLOAT64);
    return cur_node;
}

Node *parse_integer(Parser *parser) {
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
    Node *cur_node = new_Integer(parser->lex->value, parser->lex->val_len, parser->lex->line);
    eattok(parser, TOK_INT64);
    //printf("integer. type: %s, value: %s\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex->value);
    return cur_node;
}

Node *parse_boolean(Parser *parser) {
    Node *cur_node = new_Boolean(parser->lex->value, parser->lex->val_len, parser->lex->line);
    eattok(parser, TOK_BOOL);
    return cur_node;
}

Node *parse_string(Parser *parser) {
    puts("string literal");
    Node *cur_node = new_String(parser->lex->value, parser->lex->val_len, parser->lex->line);
    puts("got it");
    eattok(parser, TOK_STR);
    puts("return");
    return cur_node;
}

// parse list and map literals
Node *parse_collection(Parser *parser) {
    eattok(parser, TOK_LSQB);
    Node *keys = new_Block(parser->lex->line);
    Node *vals = new_Block(parser->lex->line); // free if we have list.

    // empty map
    if (curtok(parser) == TOK_RARR) {
        eattok(parser, TOK_RARR);
        eattok(parser, TOK_RSQB);
        return new_Map(keys, vals, parser->lex->line);
    }

    // empty list
    if (curtok(parser) == TOK_RSQB) {
        node_del(vals);
        eattok(parser, TOK_RSQB);
        return new_List(keys, parser->lex->line);
    }

    block_append(keys, parse_expr(parser));

    // non-empty map
    if (curtok(parser) == TOK_RARR) {
        eattok(parser, TOK_RARR);
        block_append(vals, parse_expr(parser));
        while (curtok(parser) == TOK_COMMA) {
            eattok(parser, TOK_COMMA);
            block_append(keys, parse_expr(parser));
            eattok(parser, TOK_RARR);
            block_append(vals, parse_expr(parser));
        }
        eattok(parser, TOK_RSQB);
        return new_Map(keys, vals, parser->lex->line);
    }

    // non-empty list
    node_del(vals);
    while (curtok(parser) == TOK_COMMA) {
        eattok(parser, TOK_COMMA);
        block_append(keys, parse_expr(parser));
    }
    eattok(parser, TOK_RSQB);
    return new_List(keys, parser->lex->line);

}