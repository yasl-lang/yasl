#include <compiler/lexer/lexer.h>
#include "../lexer/lexer.h"
#include "parser.h"
#include <inttypes.h>

int peof(const Parser *const parser) {
    return parser->lex->type == T_EOF;
}

//NOTE: keep this updated alongside token.h
static inline int tok_isaugmented(const Token t) {
    // ^=, *=, /=, //=,
    // %=, +=, -=, >>=, <<=,
    // ||=, |||=, &=, **=, |=,
    // ??=
    return t == T_CARETEQ || t == T_STAREQ || t == T_SLASHEQ || t == T_DSLASHEQ ||
           t == T_MOD || t == T_PLUSEQ || t == T_MINUSEQ || t == T_DGTEQ || t == T_DLTEQ ||
           t == T_DBAREQ || t == T_DAMPEQ || t == T_TILDEEQ || t == T_AMPEQ || t == T_DSTAREQ || t == T_BAREQ ||
           t == T_DQMARKEQ;
}

static inline Token curtok(const Parser *const parser) {
    return parser->lex->type;
}

Parser *parser_new(Lexer *lex) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lex = lex;
    return parser;
};

void parser_del(Parser *const parser) {
    lex_del(parser->lex);
    free(parser);
};

Token eattok(const Parser *const parser, const Token token) {
    if (curtok(parser) != token) {
        printf("ParsingError: Expected %s, got %s, in line %d\n", YASL_TOKEN_NAMES[token], YASL_TOKEN_NAMES[curtok(parser)], parser->lex->line);
        exit(EXIT_FAILURE);
    }
    gettok(parser->lex);
    return token;
}

Node *parse(const Parser *const parser) {
    return parse_program(parser);
}

static Node *parse_program(const Parser *const parser) {
    //YASL_DEBUG_LOG("parse. type: %s, ", YASL_TOKEN_NAMES[curtok(parser)]);
    //YASL_DEBUG_LOG("value: %s\n", parser->lex->value);
    YASL_TRACE_LOG("parsing statement in line %d\n", parser->lex->line);
    switch (curtok(parser)) {
        case T_PRINT:
            eattok(parser, T_PRINT);
            return new_Print(parse_expr(parser), parser->lex->line);
        case T_FN: return parse_fn(parser);
        case T_RET:
            eattok(parser, T_RET);
            return new_Return(parse_expr(parser), parser->lex->line);
        case T_LET: return parse_let(parser);
        case T_FOR: return parse_for(parser);
        case T_WHILE: return parse_while(parser);
        case T_BREAK:
            eattok(parser, T_BREAK);
            return new_Break(parser->lex->line);
        case T_CONT:
            eattok(parser, T_CONT);
            return new_Continue(parser->lex->line);
        case T_LBRC: return parse_block(parser);
        case T_IF: return parse_if(parser);
        case T_ELSEIF:
        case T_ELSE:
            printf("ParsingError: `%s` without previous `if`\n", YASL_TOKEN_NAMES[curtok(parser)]);
            exit(EXIT_FAILURE);
        default: return new_ExprStmt(parse_expr(parser), parser->lex->line);
    }
}

static Node *parse_fn(const Parser *const parser) {
    YASL_TRACE_LOG("parsing fn in line %d\n", parser->lex->line);
    eattok(parser, T_FN);
    char *name = parser->lex->value;
    int64_t name_len = parser->lex->val_len;
    eattok(parser, T_ID);
    eattok(parser, T_LPAR);
    Node *block = new_Body(parser->lex->line);
    while (curtok(parser) == T_ID) {
        body_append(block, parse_id(parser));
        if (curtok(parser) == T_COMMA) eattok(parser, T_COMMA);
        else break;
    }
    eattok(parser, T_RPAR);
    eattok(parser, T_LBRC);
    Node *body = new_Body(parser->lex->line);
    while (curtok(parser) != T_RBRC) {
        body_append(body, parse_program(parser));
        eattok(parser, T_SEMI);
    }
    eattok(parser, T_RBRC);
    return new_FnDecl(block, body, name, name_len, parser->lex->line);
}

static Node *parse_let(const Parser *const parser) {
    YASL_TRACE_LOG("parsing let in line %d\n", parser->lex->line);
    eattok(parser, T_LET);
    if (curtok(parser) == T_CONST) {
        eattok(parser, T_CONST);
        char *name = parser->lex->value;
        int64_t name_len = parser->lex->val_len;
        eattok(parser, T_ID);
        eattok(parser, T_EQ);
        Node *expr = parse_expr(parser);
        return new_Const(name, name_len, expr, parser->lex->line);
    } else {
        char *name = parser->lex->value;
        int64_t name_len = parser->lex->val_len;
        int64_t line = parser->lex->line;
        eattok(parser, T_ID);
        if (curtok(parser) != T_EQ) return new_Let(name, name_len, NULL, line);
        eattok(parser, T_EQ);
        return new_Let(name, name_len, parse_expr(parser), parser->lex->line);
    }
}

static Node *parse_block(const Parser *const parser) {
    YASL_TRACE_LOG("parsing block in line %d\n", parser->lex->line);
    eattok(parser, T_LBRC);
    Node *block = new_Block(new_Body(parser->lex->line), parser->lex->line);
    while (curtok(parser) != T_RBRC) {
        body_append(block->children[0], parse_program(parser));
        if (curtok(parser) == T_SEMI) eattok(parser, T_SEMI);
        else if (curtok(parser) != T_RBRC) {
            printf("ParsingError: expected newline or `}`, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, T_RBRC);
    return block;
}

static Node *parse_for(const Parser *const parser) {
    /* Currently only implements case:
     *
     * for let x in y { ... }
     *
     */
    eattok(parser, T_FOR);
    eattok(parser, T_LET);
    Node *var = parse_id(parser);
    eattok(parser, T_IN);
    Node *collection = parse_expr(parser);
    eattok(parser, T_LBRC);
    Node *body = new_Body(parser->lex->line);
    while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
        body_append(body, parse_program(parser));
        if (curtok(parser) == T_SEMI) eattok(parser, T_SEMI);
        else if (curtok(parser) != T_RBRC) {
            printf("ParsingError: expected newline or `}`, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, T_RBRC);
    return new_ForIter(var, collection, body, parser->lex->line);
}

static Node *parse_while(const Parser *const parser) {
    YASL_TRACE_LOG("parsing while in line %d\n", parser->lex->line);
    eattok(parser, T_WHILE);
    Node *cond = parse_expr(parser);
    eattok(parser, T_LBRC);
    Node *body = new_Body(parser->lex->line);
    while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
        body_append(body, parse_program(parser));
        if (curtok(parser) == T_SEMI) eattok(parser, T_SEMI);
        else if (curtok(parser) != T_RBRC) {
            printf("ParsingError: expected newline or `}`, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, T_RBRC);
    return new_While(cond, body, parser->lex->line);
}

static Node *parse_if(const Parser *const parser) {
    YASL_TRACE_LOG("parsing if in line %d\n", parser->lex->line);
    if (curtok(parser) == T_IF) eattok(parser, T_IF);
    else if (curtok(parser) == T_ELSEIF) eattok(parser, T_ELSEIF);
    else {
        printf("ParsingError: Expected if or elseif, got %s\n", YASL_TOKEN_NAMES[curtok(parser)]);
        exit(EXIT_FAILURE);
    }
    Node *cond = parse_expr(parser);
    eattok(parser, T_LBRC);
    Node *then_block = new_Body(parser->lex->line);
    while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
        body_append(then_block, parse_program(parser));
        if (curtok(parser) == T_SEMI) eattok(parser, T_SEMI);
        else if (curtok(parser) != T_RBRC) {
            printf("ParsingError: in line %d: expected newline or `}`, got `%s`.\n", parser->lex->line, YASL_TOKEN_NAMES[curtok(parser)]);
            exit(EXIT_FAILURE);
        }
    }
    eattok(parser, T_RBRC);
    if (curtok(parser) != T_ELSE && curtok(parser) != T_ELSEIF) {
        YASL_DEBUG_LOG("%s\n", "no else");
        return new_If(cond, then_block, NULL, parser->lex->line);
    }
    // TODO: eat semi
    if (curtok(parser) == T_ELSEIF) {
        YASL_DEBUG_LOG("%s\n", "elseif");
        return new_If(cond, then_block, parse_if(parser), parser->lex->line);
    }
    if (curtok(parser) == T_ELSE) {
        YASL_DEBUG_LOG("%s\n", "else");
        eattok(parser, T_ELSE);
        eattok(parser, T_LBRC);
        Node *else_block = new_Body(parser->lex->line);
        while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
            body_append(else_block, parse_program(parser));
            if (curtok(parser) == T_SEMI) eattok(parser, T_SEMI);
            else if (curtok(parser) != T_RBRC) {
                printf("ParsingError: in line %d: expected newline or `}`, got `%s`.\n", parser->lex->line, YASL_TOKEN_NAMES[curtok(parser)]);
                exit(EXIT_FAILURE);
            }
        }
        eattok(parser, T_RBRC);
        return new_If(cond, then_block, else_block, parser->lex->line);
    }
    printf("ParsingError: expected newline, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
    exit(EXIT_FAILURE);

}

static Node *parse_expr(const Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "parsing expression.");
    return parse_assign(parser);
}

static Node *parse_assign(const Parser *const parser) {
    YASL_TRACE_LOG("parsing = in line %d\n", parser->lex->line);
    Node *cur_node = parse_ternary(parser);
    if (curtok(parser) == T_EQ) {
        eattok(parser, T_EQ);
        if (cur_node->nodetype == N_VAR) {
            Node *assign_node = new_Assign(cur_node->name, cur_node->name_len, parse_assign(parser), parser->lex->line);
            free(cur_node);
            return assign_node;
        } else if (cur_node->nodetype == N_GET) {
            Node *left = cur_node->children[0];
            Node *block = new_Body(parser->lex->line);
            body_append(block, cur_node->children[1]);
            body_append(block, parse_expr(parser));
            free(cur_node->children);
            free(cur_node);
            return new_Set(left, block->children[0], block->children[1], parser->lex->line);
        } else {
            printf("SyntaxError: in line %d: invalid lvalue.\n", parser->lex->line);
            exit(EXIT_FAILURE);
        }
     // TODO: add indexing case
    } else if (tok_isaugmented(curtok(parser))) {
        Token op = eattok(parser, curtok(parser)) - 1; // relies on enum
        if (cur_node->nodetype == N_VAR) {
            char *name = cur_node->name;
            int64_t name_len = cur_node->name_len;
            Node *tmp = node_clone(cur_node);
            free(cur_node);
            return new_Assign(name, name_len, new_BinOp(op, tmp, parse_assign(parser), parser->lex->line), parser->lex->line);
        } else if (cur_node->nodetype == N_GET) {
            Node *left = cur_node->children[0];
            Node *block = new_Body(parser->lex->line);
            body_append(block, cur_node->children[1]);
            body_append(block, new_BinOp(op, node_clone(cur_node), parse_expr(parser), parser->lex->line));
            free(cur_node->children);
            free(cur_node);
            return new_Set(left, block->children[0], block->children[1], parser->lex->line);
        } else {
            printf("SyntaxError: in line %d: invalid lvalue.\n", parser->lex->line);
            exit(EXIT_FAILURE);
        }
        // TODO: add indexing case
    }
    return cur_node;
}

static Node *parse_ternary(const Parser *const parser) {
    YASL_TRACE_LOG("parsing ?: in line %d\n", parser->lex->line);
    Node *cur_node = parse_or(parser);
    if (curtok(parser) == T_DQMARK) {
        eattok(parser, T_DQMARK);
        return new_BinOp(T_DQMARK, cur_node, parse_ternary(parser), parser->lex->line);
    } else if (curtok(parser) == T_QMARK) {
        eattok(parser, T_QMARK);
        Node *left = parse_ternary(parser);
        eattok(parser, T_COLON);
        Node *right = parse_ternary(parser);
        return new_TriOp(T_QMARK, cur_node, left, right, parser->lex->line);
    }
    return cur_node;
}

static Node *parse_or(const Parser *const parser) {
    YASL_TRACE_LOG("parsing || in line %d\n", parser->lex->line);
    Node *cur_node = parse_and(parser);
    while (curtok(parser) == T_DBAR) {
        eattok(parser, T_DBAR);
        cur_node = new_BinOp(T_DBAR, cur_node, parse_and(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_and(const Parser *const parser) {
    YASL_TRACE_LOG("parsing && in line %d\n", parser->lex->line);
    Node *cur_node = parse_equals(parser);
    while (curtok(parser) == T_DAMP) {
        eattok(parser, T_DAMP);
        cur_node = new_BinOp(T_DAMP, cur_node, parse_equals(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_equals(const Parser *const parser) {
    YASL_TRACE_LOG("parsing == in line %d\n", parser->lex->line);
    Node *cur_node = parse_comparator(parser);
    while (curtok(parser) == T_DEQ || curtok(parser) == T_BANGEQ ||
            curtok(parser) == T_TEQ || curtok(parser) == T_BANGDEQ) {
        Token op = eattok(parser, curtok(parser));
        cur_node = new_BinOp(op, cur_node, parse_comparator(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_comparator(const Parser *const parser) {
    YASL_TRACE_LOG("parsing > in line %d\n", parser->lex->line);
    Node *cur_node = parse_concat(parser);
    while (curtok(parser) == T_LT || curtok(parser) == T_GT||
            curtok(parser) == T_GTEQ || curtok(parser) == T_LTEQ) {
        Token op = eattok(parser, curtok(parser));
        cur_node = new_BinOp(op, cur_node, parse_concat(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_concat(const Parser *const parser) {
    YASL_TRACE_LOG("parsing ~ in line %d\n", parser->lex->line);
    Node *cur_node = parse_bor(parser);
    if (curtok(parser) == T_TILDE) {
        eattok(parser, T_TILDE);
        return new_BinOp(T_TILDE, cur_node, parse_concat(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_bor(const Parser *const parser) {
    YASL_TRACE_LOG("parsing | in line %d\n", parser->lex->line);
    Node *cur_node = parse_bxor(parser);
    while (curtok(parser) == T_BAR) {
        eattok(parser, T_BAR);
        cur_node = new_BinOp(T_BAR, cur_node, parse_bxor(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_bxor(const Parser *const parser) {
    YASL_TRACE_LOG("parsing ^ in line %d\n", parser->lex->line);
    Node *cur_node = parse_band(parser);
    while (curtok(parser) == T_CARET) {
        eattok(parser, T_CARET);
        cur_node = new_BinOp(T_CARET, cur_node, parse_band(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_band(const Parser *const parser) {
    YASL_TRACE_LOG("parsing & in line %d\n", parser->lex->line);
    Node *cur_node = parse_bshift(parser);
    while (curtok(parser) == T_AMP) {
        eattok(parser, T_AMP);
        cur_node = new_BinOp(T_AMP, cur_node, parse_bshift(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_bshift(const Parser *const parser) {
    YASL_TRACE_LOG("parsing >> in line %d\n", parser->lex->line);
    Node *cur_node = parse_add(parser);
    while (curtok(parser) == T_DGT || curtok(parser) == T_DLT) {
        Token op = eattok(parser, curtok(parser));
        cur_node = new_BinOp(op, cur_node, parse_add(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_add(const Parser *const parser) {
    YASL_TRACE_LOG("parsing + in line %d\n", parser->lex->line);
    Node *cur_node = parse_multiply(parser);
    while (curtok(parser) == T_PLUS || curtok(parser) == T_MINUS) {
        Token op = eattok(parser, curtok(parser));
        cur_node = new_BinOp(op, cur_node, parse_multiply(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_multiply(const Parser *const parser) {
    YASL_TRACE_LOG("parsing * in line %d\n", parser->lex->line);
    Node *cur_node = parse_unary(parser);
    while (curtok(parser) == T_STAR || curtok(parser) == T_SLASH ||
            curtok(parser) == T_DSLASH || curtok(parser) == T_MOD) {
        Token op = eattok(parser, curtok(parser));
        cur_node = new_BinOp(op, cur_node, parse_unary(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_unary(const Parser *const parser) {
    YASL_TRACE_LOG("parsing ! in line %d\n", parser->lex->line);
    if (curtok(parser) == T_PLUS || curtok(parser) == T_MINUS || curtok(parser) == T_BANG ||
     curtok(parser) == T_CARET ||curtok(parser) == T_HASH) {
        Token op = eattok(parser, curtok(parser));
        return new_UnOp(op, parse_unary(parser), parser->lex->line);
    } else {
        return parse_power(parser);
    }
}

static Node *parse_power(const Parser *const parser) {
    YASL_TRACE_LOG("parsing ** in line %d\n", parser->lex->line);
    Node *cur_node = parse_call(parser);
    if (curtok(parser) == T_DSTAR) {
        eattok(parser, T_DSTAR);
        return new_BinOp(T_DSTAR, cur_node, parse_unary(parser), parser->lex->line);
    }
    return cur_node;
}

static Node *parse_call(const Parser *const parser) {
    Node *cur_node = parse_constant(parser);
    while (curtok(parser) == T_LSQB || curtok(parser) == T_DOT ||
           curtok(parser) == T_LPAR || curtok(parser) == T_DCOLON) {
        if (curtok(parser) == T_DCOLON) {
            eattok(parser, T_DCOLON);
            Node *right = parse_constant(parser);
            if (right->nodetype != N_VAR) {
                printf("SyntaxError: in line %d: Invalid method call.\n", parser->lex->line);
                exit(EXIT_FAILURE);
            }

            Node *block = new_Body(parser->lex->line);
            body_append(block, node_clone(cur_node));

            right->nodetype = N_STR;
            cur_node = new_Get(cur_node, right, parser->lex->line);


            cur_node = new_Call(block, cur_node, parser->lex->line);
            eattok(parser, T_LPAR);
            while (curtok(parser) != T_RPAR && curtok(parser) != T_EOF) {
                body_append(cur_node->children[0], parse_expr(parser));
                if (curtok(parser) != T_COMMA) break;
                eattok(parser, T_COMMA);
            }
            eattok(parser, T_RPAR);

        } else if (curtok(parser) == T_DOT) {
            eattok(parser, T_DOT);
            Node *right = parse_constant(parser);
            if (right->nodetype == N_CALL) {
                cur_node = new_Set(cur_node, right->children[0]->children[0], right->children[0]->children[1], parser->lex->line);
                free(right);
            } else if (right->nodetype == N_VAR) {
                right->nodetype = N_STR;
                cur_node = new_Get(cur_node, right, parser->lex->line);
            } else {
                printf("SyntaxError: in line %d: Invalid member access.\n", parser->lex->line);
                exit(EXIT_FAILURE);
            }
        } else if (curtok(parser) == T_LSQB) {
            eattok(parser, T_LSQB);
            cur_node = new_Get(cur_node, parse_expr(parser), parser->lex->line);
            eattok(parser, T_RSQB);
        } else if (curtok(parser) == T_LPAR) {
            YASL_TRACE_LOG("%s\n", "Parsing function call");
            cur_node = new_Call(new_Body(parser->lex->line), cur_node, parser->lex->line);
            eattok(parser, T_LPAR);
            while (curtok(parser) != T_RPAR && curtok(parser) != T_EOF) {
                body_append(cur_node->children[0], parse_expr(parser));
                if (curtok(parser) != T_COMMA) break;
                eattok(parser, T_COMMA);
            }
            eattok(parser, T_RPAR);
        }
    }
    return cur_node;

}

static Node *parse_constant(const Parser *const parser) {
    switch (curtok(parser)) {
        case T_ID: return parse_id(parser);
        case T_LPAR:
            eattok(parser, T_LPAR);
            Node *expr = parse_expr(parser);
            eattok(parser, T_RPAR);
            return expr;
        case T_LSQB: return parse_collection(parser);
        case T_STR: return parse_string(parser);
        case T_INT64: return parse_integer(parser);
        case T_FLOAT64: return parse_float(parser);
        case T_BOOL: return parse_boolean(parser);
        case T_UNDEF: return parse_undef(parser);
        // handle invalid expressions with sensible error messages.
        case T_PRINT:
        case T_FN:
        case T_LET:
        case T_WHILE:
        case T_BREAK:
        case T_RET:
        case T_CONT:
        case T_IF:
        case T_ELSEIF:
        case T_ELSE:
            printf("ParsingError in line %" PRId64 ": expected expression, got `%s`\n", parser->lex->line, YASL_TOKEN_NAMES[curtok(parser)]);
            exit(EXIT_FAILURE);
        default:
            printf("ParsingError: Invalid expression in line %" PRId64 " (%s).\n", parser->lex->line, YASL_TOKEN_NAMES[curtok(parser)]);
            exit(EXIT_FAILURE);
    }
}

static Node *parse_id(const Parser *const parser) {
    char *name = parser->lex->value;
    int64_t name_len = parser->lex->val_len;
    int64_t line = parser->lex->line;
    eattok(parser, T_ID);
    YASL_TRACE_LOG("%s\n", "Parsing variable");
    Node *cur_node = new_Var(name, name_len, line);
    return cur_node;
}

static Node *parse_undef(const Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing undef");
    Node *cur_node = new_Undef(parser->lex->line);
    eattok(parser, T_UNDEF);
    return cur_node;
}

static Node *parse_float(const Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing float64");
    Node* cur_node = new_Float(parser->lex->value, parser->lex->val_len, parser->lex->line);
    eattok(parser, T_FLOAT64);
    return cur_node;
}

static Node *parse_integer(const Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing int64");
    Node *cur_node = new_Integer(parser->lex->value, parser->lex->val_len, parser->lex->line);
    eattok(parser, T_INT64);
    return cur_node;
}

static Node *parse_boolean(const Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing bool");
    Node *cur_node = new_Boolean(parser->lex->value, parser->lex->val_len, parser->lex->line);
    eattok(parser, T_BOOL);
    return cur_node;
}

static Node *parse_string(const Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing str");
    Node *cur_node = new_String(parser->lex->value, parser->lex->val_len, parser->lex->line);
    eattok(parser, T_STR);
    return cur_node;
}

// parse list and table literals
static Node *parse_collection(const Parser *const parser) {
    eattok(parser, T_LSQB);
    Node *keys = new_Body(parser->lex->line);

    // empty table
    if (curtok(parser) == T_COLON) {
        YASL_TRACE_LOG("%s\n", "Parsing table");
        eattok(parser, T_COLON);
        eattok(parser, T_RSQB);
        return new_Table(keys, parser->lex->line);
    }

    // empty list
    if (curtok(parser) == T_RSQB) {
        YASL_TRACE_LOG("%s\n", "Parsing list");
        eattok(parser, T_RSQB);
        return new_List(keys, parser->lex->line);
    }

    body_append(keys, parse_expr(parser));

    // non-empty table
    if (curtok(parser) == T_COLON) {
        YASL_TRACE_LOG("%s\n", "Parsing table");
        eattok(parser, T_COLON);
        body_append(keys, parse_expr(parser));

        if (curtok(parser) == T_FOR) {
            eattok(parser, T_FOR);
            eattok(parser, T_LET);
            Node *var = parse_id(parser);
            eattok(parser, T_IN);
            Node *collection = parse_expr(parser);
            Node *cond = NULL;
            if (curtok(parser) == T_IF) {
                eattok(parser, T_IF);
                cond = parse_expr(parser);
            }
            eattok(parser, T_RSQB);
            Node *table_comp = new_TableComp(cond ? new_If(cond, keys/*->children[0]*/, NULL, parser->lex->line) : keys/*->children[0]*/, var, collection, parser->lex->line);
            return table_comp;
        }
        while (curtok(parser) == T_COMMA) {
            eattok(parser, T_COMMA);
            body_append(keys, parse_expr(parser));
            eattok(parser, T_COLON);
            body_append(keys, parse_expr(parser));
        }
        eattok(parser, T_RSQB);
        return new_Table(keys, parser->lex->line);
    }

    // non-empty list
    if (curtok(parser) == T_FOR) {
        eattok(parser, T_FOR);
        eattok(parser, T_LET);
        Node *var = parse_id(parser);
        eattok(parser, T_IN);
        Node *collection = parse_expr(parser);
        Node *cond = NULL;
        if (curtok(parser) == T_IF) {
            eattok(parser, T_IF);
            cond = parse_expr(parser);
        }
        eattok(parser, T_RSQB);
        Node *table_comp = new_ListComp(cond ? new_If(cond, keys->children[0], NULL, parser->lex->line) : keys->children[0], var, collection, parser->lex->line);
        free(keys);
        return table_comp;
    } else {
        while (curtok(parser) == T_COMMA) {
            YASL_TRACE_LOG("%s\n", "Parsing list");
            eattok(parser, T_COMMA);
            body_append(keys, parse_expr(parser));
        }
        eattok(parser, T_RSQB);
        return new_List(keys, parser->lex->line);
    }
}