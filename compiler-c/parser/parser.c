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
    } else if (parser->lex->type == TOK_WHILE) {
        return parse_while(parser);
    } else if (parser->lex->type == TOK_BREAK) {
        eattok(parser, TOK_BREAK);
        return new_Break();
    } else if (parser->lex->type == TOK_CONT) {
        eattok(parser, TOK_CONT);
        return new_Continue();
    } else if (parser->lex->type == TOK_IF) {
        return parse_if(parser);
    } else if (parser->lex->type == TOK_ELSEIF) {
        puts("ParsingError: elseif without previous if");
        exit(EXIT_FAILURE);
    } else if (parser->lex->type == TOK_ELSE) {
        puts("ParsingError: else without previous if");
        exit(EXIT_FAILURE);
    } else return new_ExprStmt(parse_expr(parser), parser->lex->line);
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
    while (parser->lex->type != TOK_RBRC && parser->lex->type != TOK_EOF) {
        block_append(body, parse_program(parser));
        if (parser->lex->type == TOK_SEMI) eattok(parser, TOK_SEMI);
        else if (parser->lex->type != TOK_RBRC) {
            puts("expected semicolon/newline or right brace");
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, TOK_RBRC);
    return new_While(cond, body);
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
    if (parser->lex->type == TOK_IF) eattok(parser, TOK_IF);
    else if (parser->lex->type == TOK_ELSEIF) eattok(parser, TOK_ELSEIF);
    else {
        printf("ParsingError: Expected if or elseif, got %s\n", YASL_TOKEN_NAMES[parser->lex->type]);
        exit(EXIT_FAILURE);
    }
    Node *cond = parse_expr(parser);
    eattok(parser, TOK_LBRC);
    Node *then_block = new_Block(parser->lex->line);
    while (parser->lex->type != TOK_RBRC && parser->lex->type != TOK_EOF) {
        block_append(then_block, parse_program(parser));
        if (parser->lex->type == TOK_SEMI) eattok(parser, TOK_SEMI);
        else if (parser->lex->type != TOK_RBRC) {
            puts("expected semicolon/newline or right brace");
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, TOK_RBRC);
    if (parser->lex->type != TOK_ELSE && parser->lex->type != TOK_ELSEIF) {
        puts("No Else");
        return new_If(cond, then_block, NULL);
    }
    // TODO: eat semi
    if (parser->lex->type == TOK_ELSEIF) {
        puts("ElseIf");
        return new_If(cond, then_block, parse_if(parser));
    }
    if (parser->lex->type == TOK_ELSE) {
        puts("Else");
        eattok(parser, TOK_ELSE);
        eattok(parser, TOK_LBRC);
        Node *else_block = new_Block(parser->lex->line);
        while (parser->lex->type != TOK_RBRC && parser->lex->type != TOK_EOF) {
            block_append(else_block, parse_program(parser));
            if (parser->lex->type == TOK_SEMI) eattok(parser, TOK_SEMI);
            else if (parser->lex->type != TOK_RBRC) {
                puts("expected semicolon/newline or right brace");
                exit(EXIT_FAILURE);
            }
        }
        eattok(parser, TOK_RBRC);
        return new_If(cond, then_block, else_block);
    }
    printf("ParsingError: expected newline or semicolon, got %s\n", YASL_TOKEN_NAMES[parser->lex->type]);
    exit(EXIT_FAILURE);

}

Node *parse_expr(Parser *parser) {
    return parse_assign(parser);
}

Node *parse_assign(Parser *parser) {
    Node *cur_node = parse_ternary(parser);
    if (parser->lex->type == TOK_EQ) { // || parser->lex->type == TOK_DLT)
        eattok(parser, TOK_EQ);
        if (cur_node->nodetype == NODE_VAR) {
            Node *assign_node = new_Assign(cur_node->name, cur_node->name_len, parse_assign(parser));
            node_del(cur_node);
            return assign_node;
        }
        // TODO: add indexing case
    } else if (isaugmented(parser->lex->type)) {
        Token op = eattok(parser, parser->lex->type) - 1; // relies on enum
        if (cur_node->nodetype == NODE_VAR) {
            return new_Assign(cur_node->name, cur_node->name_len, new_BinOp(op, cur_node, parse_assign(parser)));
        }
        // TODO: add indexing case
        /*
         * elif self.current_token.value in ("^=", "*=", "/=", "//=", "%=", "+=", "-=", ">>=", "<<=", \
                                          "||=", "|||=", "&=", "~=", "|=", "??="):
            token = self.eat(TokenTypes.OP)
            if isinstance(name, Var) or isinstance(name, Index):
                right = self.expr()
                return Assign(name, BinOp(Token(TokenTypes.OP, token.value.rstrip("="), token.line), name, right))
            else:
                raise Exception("Invalid assignment target.")
         */
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
    else if (parser->lex->type == TOK_LSQB) return parse_collection(parser);
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
        free(name);
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

// parse list and map literals
Node *parse_collection(Parser *parser) {
    /*
    elif self.current_token.type is TokenTypes.LBRACK:
        ls = self.eat(TokenTypes.LBRACK)
        keys = []
        vals = []
        if self.current_token.type is TokenTypes.RARROW:
            self.eat(TokenTypes.RARROW)
            self.eat(TokenTypes.RBRACK)
            return Hash(ls, keys, vals)
        if self.current_token.type is not TokenTypes.RBRACK:
            keys.append(self.expr())
            if self.current_token.type is TokenTypes.RARROW:
                self.eat(TokenTypes.RARROW)
                vals.append(self.expr())
                while self.current_token.type is TokenTypes.COMMA and self.current_token.type is not TokenTypes.EOF:
                    self.eat(TokenTypes.COMMA)
                    keys.append(self.expr())
                    self.eat(TokenTypes.RARROW)
                    vals.append(self.expr())
                self.eat(TokenTypes.RBRACK)
                return Hash(ls, keys, vals)
            else:
                while self.current_token.type is TokenTypes.COMMA and self.current_token.type is not TokenTypes.EOF:
                    self.eat(TokenTypes.COMMA)
                    keys.append(self.expr())
                self.eat(TokenTypes.RBRACK)
                return List(ls, keys)
        self.eat(TokenTypes.RBRACK)
        return List(ls, keys)
    */
    eattok(parser, TOK_LSQB);
    Node *keys = new_Block(parser->lex->line);
    Node *vals = new_Block(parser->lex->line); // free if we have list.

    if (parser->lex->type == TOK_RARR) {
        eattok(parser, TOK_RARR);
        eattok(parser, TOK_RSQB);
        return new_Map(keys, vals);
    }

    if (parser->lex->type == TOK_RSQB) {
        node_del(vals);
        eattok(parser, TOK_RSQB);
        return new_List(keys);
    }

    // TODO: handle non-emtpy case

}