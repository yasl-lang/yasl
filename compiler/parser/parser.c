#include "parser.h"

#include <inttypes.h>

#include "ast.h"
#include "lexer.h"
#include "middleend.h"
#include "yasl_conf.h"
#include "yasl_error.h"
#include "yasl_include.h"

static struct Node *parse_program(Parser *parser);
static struct Node *parse_const(Parser *parser);
static struct Node *parse_fn(Parser *parser);
static struct Node *parse_for(Parser *parser);
static struct Node *parse_while(Parser *parser);
static struct Node *parse_if(Parser *parser);
static struct Node *parse_expr(Parser *parser);
static struct Node *parse_assign(Parser *parser);
static struct Node *parse_ternary(Parser *parser);
static struct Node *parse_undef_or(Parser *parser);
static struct Node *parse_or(Parser *parser);
static struct Node *parse_and(Parser *parser);
static struct Node *parse_bor(Parser *parser);
static struct Node *parse_bxor(Parser *parser);
static struct Node *parse_band(Parser *parser);
static struct Node *parse_equals(Parser *parser);
static struct Node *parse_comparator(Parser *parser);
static struct Node *parse_concat(Parser *parser);
static struct Node *parse_bshift(Parser *parser);
static struct Node *parse_add(Parser *parser);
static struct Node *parse_multiply(Parser *parser);
static struct Node *parse_unary(Parser *parser);
static struct Node *parse_power(Parser *parser);
static struct Node *parse_call(Parser *parser);
static struct Node *parse_constant(Parser *parser);
static struct Node *parse_id(Parser *parser);
static struct Node *parse_undef(Parser *parser);
static struct Node *parse_float(Parser *parser);
static struct Node *parse_integer(Parser *parser);
static struct Node *parse_boolean(Parser *parser);
static struct Node *parse_string(Parser *parser);
static struct Node *parse_table(Parser *parser);
static struct Node *parse_collection(Parser *parser);

int peof(const Parser *const parser) {
    return parser->lex.type == T_EOF;
}

//NOTE: keep this updated alongside token.h
static inline int tok_isaugmented(const Token t) {
    // ^=, *=, /=, //=,
    // %=, +=, -=, >>=, <<=,
    // ||=, |||=, &=, **=, |=,
    // ??=
    return t == T_CARETEQ || t == T_STAREQ || t == T_SLASHEQ || t == T_DSLASHEQ ||
           t == T_MOD || t == T_PLUSEQ || t == T_MINUSEQ || t == T_DGTEQ || t == T_DLTEQ ||
           t == T_DBAREQ || t == T_DAMPEQ || t == T_TILDEEQ || t == T_AMPEQ || t == T_AMPCARETEQ || t == T_DSTAREQ || t == T_BAREQ ||
           t == T_DQMARKEQ;
}

static inline Token curtok(const Parser *const parser) {
    return parser->lex.type;
}

void parser_cleanup(Parser *const parser) {
	lex_cleanup(&parser->lex);
	//free(parser);
}

static struct Node *handle_error(Parser *parser) {
    parser->status = YASL_SYNTAX_ERROR;
    while (curtok(parser) != T_SEMI) eattok(parser, curtok(parser));
    return NULL;
}

Token eattok(Parser *const parser, const Token token) {
    if (curtok(parser) != token) {
        if (curtok(parser) == T_UNKNOWN) {
            parser->status = parser->lex.status;
        } else {
            YASL_PRINT_ERROR_SYNTAX("Expected %s, got %s, in line %zd\n", YASL_TOKEN_NAMES[token],
                   YASL_TOKEN_NAMES[curtok(parser)], parser->lex.line);
            parser->status = YASL_SYNTAX_ERROR;
        }
        while (!TOKEN_MATCHES(parser, T_SEMI, T_EOF)) gettok(&parser->lex);
    } else {
        gettok(&parser->lex);
    }
    return token;
}

struct Node *parse(Parser *const parser) {
    return parse_program(parser);
}

static struct Node *parse_program(Parser *const parser) {
    //YASL_DEBUG_LOG("parse. type: %s, ", YASL_TOKEN_NAMES[curtok(parser)]);
    //YASL_DEBUG_LOG("value: %s\n", parser->lex.value);
    YASL_TRACE_LOG("parsing statement in line %zd\n", parser->lex.line);
    size_t line;
    struct Node *expr;
    switch (curtok(parser)) {
        case T_ECHO:
            eattok(parser, T_ECHO);
            return new_Print(parse_expr(parser), parser->lex.line);
        case T_FN: return parse_fn(parser);
        case T_RET:
            eattok(parser, T_RET);
            return new_Return(parse_expr(parser), parser->lex.line);
        case T_CONST: return parse_const(parser);
        case T_FOR: return parse_for(parser);
        case T_WHILE: return parse_while(parser);
        case T_BREAK:
            line = parser->lex.line;
            eattok(parser, T_BREAK);
            return new_Break(line);
        case T_CONT:
            line = parser->lex.line;
            eattok(parser, T_CONT);
            return new_Continue(line);
        case T_IF: return parse_if(parser);
        case T_ELSEIF:
        case T_ELSE:
            YASL_PRINT_ERROR_SYNTAX("`%s` without previous `if`\n", YASL_TOKEN_NAMES[curtok(parser)]);
            return handle_error(parser);
        case T_UNKNOWN:
            parser->status = parser->lex.status;
            return NULL;
        default:
            line = parser->lex.line;
            expr = parse_expr(parser);
            if (curtok(parser) == T_COLONEQ) {
                if (expr->nodetype != N_VAR) {
                    YASL_PRINT_ERROR_SYNTAX("Invalid lvalue in line %zd\n", parser->lex.line);
                    return handle_error(parser);
                }
                eattok(parser, T_COLONEQ);
                struct Node *assign_node = new_Let(expr->value.sval.str, expr->value.sval.str_len, parse_expr(parser), line);
                free(expr);
                return assign_node;
            }
            return new_ExprStmt(expr, parser->lex.line);

    }
}

static struct Node *parse_body(Parser *const parser) {
    eattok(parser, T_LBRC);
    struct Node *body = new_Body(parser->lex.line);
    while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
        body_append(&body, parse_program(parser));
        eattok(parser, T_SEMI);
    }
    eattok(parser, T_RBRC);
    return body;
}

static struct Node *parse_function_params(Parser *const parser) {
	struct Node *block = new_Body(parser->lex.line);
	while (TOKEN_MATCHES(parser, T_ID, T_CONST)) {
		if (TOKEN_MATCHES(parser, T_ID)) {
			body_append(&block, parse_id(parser));
		} else {
			eattok(parser, T_CONST);
			struct Node *cur_node = parse_id(parser);
			cur_node->nodetype = N_CONST;
			body_append(&block, cur_node);
		}
		if (curtok(parser) == T_COMMA) eattok(parser, T_COMMA);
		else break;
	}
	return block;
}

static struct Node *parse_fn(Parser *const parser) {
	YASL_TRACE_LOG("parsing fn in line %zd\n", parser->lex.line);
	eattok(parser, T_FN);
	size_t line = parser->lex.line;
	char *name = parser->lex.value;
	size_t name_len = parser->lex.val_len;
	eattok(parser, T_ID);
	eattok(parser, T_LPAR);
	struct Node *block = parse_function_params(parser);
	eattok(parser, T_RPAR);

	struct Node *body = parse_body(parser);

	char *name2 = malloc(name_len);
	memcpy(name2, name, name_len);
	return new_Let(name, name_len, new_FnDecl(block, body, name2, name_len, parser->lex.line), line);

}

static struct Node *parse_const(Parser *const parser) {
	YASL_TRACE_LOG("parsing let in line %zd\n", parser->lex.line);
	eattok(parser, T_CONST);
	char *name = parser->lex.value;
	size_t name_len = parser->lex.val_len;
	size_t line = parser->lex.line;
	if (TOKEN_MATCHES(parser, T_FN)) {
		struct Node *cur_node = parse_fn(parser);
		cur_node->nodetype = N_CONST;
		return cur_node;
	}
	eattok(parser, T_ID);
	eattok(parser, T_COLONEQ);
	struct Node *expr = parse_expr(parser);
	return new_Const(name, name_len, expr, line);
}

static struct Node *parse_let_iterate_or_let(Parser *const parser) {
    char *name = parser->lex.value;
    size_t name_len = parser->lex.val_len;
    size_t line = parser->lex.line;
    eattok(parser, T_ID);
    if (curtok(parser) == T_COLONEQ) {
        eattok(parser, T_COLONEQ);
        struct Node *expr = parse_expr(parser);
        return new_Let(name, name_len, expr, line);
    } else {
        eattok(parser, T_LEFT_ARR);
        struct Node *expr = parse_expr(parser);
        return new_LetIter(new_Var(name, name_len, line), expr, line);
    }
}

static struct Node *parse_iterate(Parser *const parser) {
    size_t line = parser->lex.line;
    struct Node *var = parse_id(parser);
    eattok(parser, T_LEFT_ARR);
    struct Node *collection = parse_expr(parser);
    return new_LetIter(var, collection, line);
}

static struct Node *parse_for(Parser *const parser) {
    eattok(parser, T_FOR);

    struct Node *iter = parse_let_iterate_or_let(parser);

    if (iter->nodetype == N_LETITER) {
        struct Node *body = parse_body(parser);
        return new_ForIter(iter, body, parser->lex.line);
    } else {
        eattok(parser, T_SEMI);
        struct Node *cond = parse_expr(parser);
        eattok(parser, T_SEMI);
        struct Node *post = parse_expr(parser);
        struct Node *body = parse_body(parser);
        struct Node *outer_body = new_Body(parser->lex.line);
        body_append(&outer_body, iter);
        body_append(&outer_body, new_While(cond, body, new_ExprStmt(post, parser->lex.line), parser->lex.line));
        struct Node *block = new_Block(outer_body, parser->lex.line);
        return block;
    }
}

static struct Node *parse_while(Parser *const parser) {
    YASL_TRACE_LOG("parsing while in line %zd\n", parser->lex.line);
    eattok(parser, T_WHILE);
    struct Node *cond = parse_expr(parser);
    struct Node *body = parse_body(parser);
    return new_While(cond, body, NULL, parser->lex.line);
}

static struct Node *parse_if(Parser *const parser) {
    YASL_TRACE_LOG("parsing if in line %zd\n", parser->lex.line);
    if (curtok(parser) == T_IF) eattok(parser, T_IF);
    else if (curtok(parser) == T_ELSEIF) eattok(parser, T_ELSEIF);
    else {
        YASL_PRINT_ERROR_SYNTAX("Expected if or elseif, got %s\n", YASL_TOKEN_NAMES[curtok(parser)]);
        return handle_error(parser);
    }
    struct Node *cond = parse_expr(parser);
    struct Node *then_block = parse_body(parser);
    if (curtok(parser) != T_ELSE && curtok(parser) != T_ELSEIF) {
        YASL_DEBUG_LOG("%s\n", "no else");
        return new_If(cond, then_block, NULL, parser->lex.line);
    }
    // TODO: eat semi
    if (curtok(parser) == T_ELSEIF) {
        YASL_DEBUG_LOG("%s\n", "elseif");
        return new_If(cond, then_block, parse_if(parser), parser->lex.line);
    }
    if (curtok(parser) == T_ELSE) {
        YASL_DEBUG_LOG("%s\n", "else");
        eattok(parser, T_ELSE);
        struct Node *else_block = parse_body(parser);
        return new_If(cond, then_block, else_block, parser->lex.line);
    }
    YASL_PRINT_ERROR_SYNTAX("Expected newline, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
    return handle_error(parser);

}

static struct Node *parse_expr(Parser *const parser) {
	YASL_TRACE_LOG("%s\n", "parsing expression.");
	struct Node *node = parse_assign(parser);
	if (node)
		fold(node);
	return node;
}

static struct Node *parse_assign(Parser *const parser) {
	YASL_TRACE_LOG("parsing = in line %zd\n", parser->lex.line);
	struct Node *cur_node = parse_ternary(parser);
	size_t line = parser->lex.line;
	if (curtok(parser) == T_EQ) {
		eattok(parser, T_EQ);
		switch (cur_node->nodetype) {
		case N_VAR: {
			struct Node *assign_node = new_Assign(cur_node->value.sval.str, cur_node->value.sval.str_len,
							      parse_assign(parser), line);
			free(cur_node);
			return assign_node;
		}
		case N_GET: {
			struct Node *left = cur_node->children[0];
			struct Node *key = cur_node->children[1];
			struct Node *val = parse_expr(parser);
			free(cur_node);
			return new_Set(left, key, val, line);
		}
		default:
			YASL_PRINT_ERROR_SYNTAX("Invalid l-value (line %zd).\n", line);
			return handle_error(parser);
		}
	} else if (tok_isaugmented(curtok(parser))) {
		Token op = eattok(parser, curtok(parser)) - 1; // relies on enum
		switch (cur_node->nodetype) {
		case N_VAR: {
			char *name = cur_node->value.sval.str;
			size_t name_len = cur_node->value.sval.str_len;
			struct Node *tmp = node_clone(cur_node);
			free(cur_node);
			return new_Assign(name, name_len, new_BinOp(op, tmp, parse_assign(parser), line), line);
		}
		case N_GET: {
			struct Node *left = cur_node->children[0];
			struct Node *block = new_Body(parser->lex.line);
			body_append(&block, cur_node->children[1]);
			body_append(&block, new_BinOp(op, node_clone(cur_node), parse_expr(parser), line));
			free(cur_node->children);
			free(cur_node);
			return new_Set(left, block->children[0], block->children[1], line);
		}
		default:
			YASL_PRINT_ERROR_SYNTAX("Invalid l-value (line %zd).\n", line);
			return handle_error(parser);
		}
	}
	return cur_node;
}

static struct Node *parse_ternary(Parser *const parser) {
    YASL_TRACE_LOG("parsing ?: in line %zd\n", parser->lex.line);
    struct Node *cur_node = parse_undef_or(parser);
    if (curtok(parser) == T_QMARK) {
        eattok(parser, T_QMARK);
        struct Node *left = parse_ternary(parser);
        eattok(parser, T_COLON);
        struct Node *right = parse_ternary(parser);
        return new_TriOp(T_QMARK, cur_node, left, right, parser->lex.line);
    }
    return cur_node;
}


#define BINOP_R(name, next, msg, ...)\
static struct Node *parse_##name(Parser *const parser) {\
	YASL_TRACE_LOG("parsing " msg " in line %zd\n", parser->lex.line);\
	struct Node *cur_node = parse_##next(parser);\
	if (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
		Token op = eattok(parser, curtok(parser));\
		return new_BinOp(op, cur_node, parse_##name(parser), parser->lex.line);\
	}\
	return cur_node;\
}

#define BINOP_L(name, next, msg, ...) \
static struct Node *parse_##name(Parser *const parser) {\
	YASL_TRACE_LOG("parsing " msg " in line %zd\n", parser->lex.line);\
	struct Node *cur_node = parse_##next(parser);\
	while (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
		Token op = eattok(parser, curtok(parser));\
		cur_node = new_BinOp(op, cur_node, parse_##next(parser), parser->lex.line);\
	}\
	return cur_node;\
}

BINOP_R(undef_or, or, "??", T_DQMARK)
BINOP_R(or, and, "||", T_DBAR)
BINOP_R(and, equals, "&&", T_DAMP)
BINOP_L(equals, comparator, "==, !=, ===, !===", T_DEQ, T_BANGEQ, T_TEQ, T_BANGDEQ)
BINOP_L(comparator, concat, "<, <=, >, >=", T_LT, T_LTEQ, T_GT, T_GTEQ)
BINOP_R(concat, bor, "~", T_TILDE)
BINOP_L(bor, bxor, "|", T_BAR)
BINOP_L(bxor, band, "^", T_CARET)
BINOP_L(band, bshift, "&, &^", T_AMP, T_AMPCARET)
BINOP_L(bshift, add, ">> and <<", T_DGT, T_DLT)
BINOP_L(add, multiply, "+ and -", T_PLUS, T_MINUS)
BINOP_L(multiply, unary, "*, %%, / and //", T_STAR, T_DSLASH, T_SLASH, T_MOD)

static struct Node *parse_unary(Parser *const parser) {
    YASL_TRACE_LOG("parsing !, -, +, ^, len in line %zd\n", parser->lex.line);
    if (curtok(parser) == T_PLUS || curtok(parser) == T_MINUS || curtok(parser) == T_BANG ||
     curtok(parser) == T_CARET ||curtok(parser) == T_LEN) {
        Token op = eattok(parser, curtok(parser));
        return new_UnOp(op, parse_unary(parser), parser->lex.line);
    } else {
        return parse_power(parser);
    }
}

static struct Node *parse_power(Parser *const parser) {
    YASL_TRACE_LOG("parsing ** in line %zd\n", parser->lex.line);
    struct Node *cur_node = parse_call(parser);
    if (TOKEN_MATCHES(parser, T_DSTAR)) {
        eattok(parser, T_DSTAR);
        return new_BinOp(T_DSTAR, cur_node, parse_unary(parser), parser->lex.line);
    }
    return cur_node;
}

static struct Node *parse_call(Parser *const parser) {
    struct Node *cur_node = parse_constant(parser);
    while (TOKEN_MATCHES(parser, T_LSQB, T_DOT, T_LPAR, T_RIGHT_ARR)) {
        if (TOKEN_MATCHES(parser, T_RIGHT_ARR)) {
		eattok(parser, T_RIGHT_ARR);
		struct Node *right = parse_constant(parser);
		if (right->nodetype != N_VAR) {
			YASL_PRINT_ERROR_SYNTAX("Invalid method call (line %zd).\n", parser->lex.line);
			return handle_error(parser);
		}

		struct Node *block = new_Body(parser->lex.line);

		cur_node = new_MethodCall(block, cur_node, right->value.sval.str, right->value.sval.str_len,
					  parser->lex.line);
		free(right);

		eattok(parser, T_LPAR);
		while (!TOKEN_MATCHES(parser, T_RPAR, T_EOF)) {
			body_append(&cur_node->children[0], parse_expr(parser));
			if (curtok(parser) != T_COMMA) break;
			eattok(parser, T_COMMA);
		}
		eattok(parser, T_RPAR);

	} else if (curtok(parser) == T_DOT) {
            eattok(parser, T_DOT);
            struct Node *right = parse_constant(parser);
            if (right->nodetype == N_CALL) {
                cur_node = new_Set(cur_node, right->children[0]->children[0], right->children[0]->children[1], parser->lex.line);
                free(right);
            } else if (right->nodetype == N_VAR) {
                right->nodetype = N_STR;
                cur_node = new_Get(cur_node, right, parser->lex.line);
            } else {
                YASL_PRINT_ERROR_SYNTAX("Invalid member access (line %zd).\n", parser->lex.line);
                return handle_error(parser);
            }
        } else if (curtok(parser) == T_LSQB) {
            eattok(parser, T_LSQB);
            cur_node = new_Get(cur_node, parse_expr(parser), parser->lex.line);
            eattok(parser, T_RSQB);
        } else if (curtok(parser) == T_LPAR) {
            YASL_TRACE_LOG("%s\n", "Parsing function call");
            cur_node = new_Call(new_Body(parser->lex.line), cur_node, parser->lex.line);
            eattok(parser, T_LPAR);
            while (!TOKEN_MATCHES(parser, T_RPAR, T_EOF)) {
                body_append(&cur_node->children[0], parse_expr(parser));
                if (curtok(parser) != T_COMMA) break;
                eattok(parser, T_COMMA);
            }
            eattok(parser, T_RPAR);
        }
    }
    return cur_node;

}

static struct Node *parse_constant(Parser *const parser) {
    switch (curtok(parser)) {
        case T_DOT:
            eattok(parser, T_DOT);
            struct Node *cur_node = new_String(parser->lex.value, parser->lex.val_len, parser->lex.line);
            eattok(parser, T_ID);
            return cur_node;
        case T_ID: return parse_id(parser);
        case T_LPAR:
            eattok(parser, T_LPAR);
            struct Node *expr = parse_expr(parser);
            eattok(parser, T_RPAR);
            return expr;
        case T_LSQB: return parse_collection(parser);
        case T_LBRC: return parse_table(parser);
        case T_STR: return parse_string(parser);
        case T_INT: return parse_integer(parser);
        case T_FLOAT: return parse_float(parser);
        case T_BOOL: return parse_boolean(parser);
        case T_UNDEF: return parse_undef(parser);
        // handle invalid expressions with sensible error messages.
        case T_ECHO:
        case T_FN:
        case T_WHILE:
        case T_BREAK:
        case T_RET:
        case T_CONT:
        case T_IF:
        case T_ELSEIF:
        case T_ELSE:
            YASL_PRINT_ERROR_SYNTAX("ParsingError in line %" PRId64 ": expected expression, got `%s`\n", parser->lex.line, YASL_TOKEN_NAMES[curtok(parser)]);
            return handle_error(parser);
        case T_UNKNOWN:
            parser->status = parser->lex.status;
            return NULL;
        default:
            YASL_PRINT_ERROR_SYNTAX("Invalid expression in line %" PRId64 " (%s).\n", parser->lex.line, YASL_TOKEN_NAMES[curtok(parser)]);
            return handle_error(parser);
    }
}

static struct Node *parse_id(Parser *const parser) {
    char *name = parser->lex.value;
    size_t name_len = parser->lex.val_len;
    size_t line = parser->lex.line;
    eattok(parser, T_ID);
    YASL_TRACE_LOG("%s\n", "Parsing variable");
    struct Node *cur_node = new_Var(name, name_len, line);
    return cur_node;
}

static struct Node *parse_undef(Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing undef");
    struct Node *cur_node = new_Undef(parser->lex.line);
    eattok(parser, T_UNDEF);
    return cur_node;
}

static double get_float(char *buffer) {
	return strtod(buffer, (char **) NULL);
}

static struct Node *parse_float(Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing float");
    struct Node* cur_node = new_Float(get_float(parser->lex.value), parser->lex.line);
    free(parser->lex.value);
    eattok(parser, T_FLOAT);
    return cur_node;
}

static yasl_int get_int(char *buffer) {
	if (strlen(buffer) < 2) {
		return strtoll(buffer, (char **) NULL, 10);
	}
	switch (buffer[1]) {
	case 'x':
		return strtoll(buffer + 2, (char **) NULL, 16);
	case 'b':
		return strtoll(buffer + 2, (char **) NULL, 2);
	default:
		return strtoll(buffer, (char **) NULL, 10);
	}
}

static struct Node *parse_integer(Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing int");
    struct Node *cur_node = new_Integer(get_int(parser->lex.value), parser->lex.line);
    free(parser->lex.value);
    eattok(parser, T_INT);
    return cur_node;
}

static struct Node *parse_boolean(Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing bool");
    struct Node *cur_node = new_Boolean(!strcmp(parser->lex.value, "true"), parser->lex.line);
    free(parser->lex.value);
    eattok(parser, T_BOOL);
    return cur_node;
}

static struct Node *parse_string(Parser *const parser) {
    YASL_TRACE_LOG("%s\n", "Parsing str");
    struct Node *cur_node = new_String(parser->lex.value, parser->lex.val_len, parser->lex.line);

    while (parser->lex.mode == L_INTERP) {
        eattok(parser, T_STR);
        eattok(parser, T_LBRC);
        struct Node *expr = parse_expr(parser);
        cur_node = new_BinOp(T_TILDE, cur_node, expr, parser->lex.line);
        if (parser->lex.c == '}') {
            parser->lex.c = fgetc(parser->lex.file);
        }
        lex_eatinterpstringbody(&parser->lex);
        struct Node *str = new_String(parser->lex.value, parser->lex.val_len, parser->lex.line);
        cur_node = new_BinOp(T_TILDE, cur_node, str, parser->lex.line);
    }

    eattok(parser, T_STR);

    return cur_node;
}


static struct Node *parse_table(Parser *const parser) {
    eattok(parser, T_LBRC);
    struct Node *keys = new_Body(parser->lex.line);

    // empty table
    if (curtok(parser) == T_RBRC) {
        YASL_TRACE_LOG("%s\n", "Parsing list");
        eattok(parser, T_RBRC);
        return new_Table(keys, parser->lex.line);
    }

    body_append(&keys, parse_expr(parser));

    // non-empty table
        YASL_TRACE_LOG("%s\n", "Parsing table");
        eattok(parser, T_COLON);
        body_append(&keys, parse_expr(parser));

        if (curtok(parser) == T_FOR) {
            eattok(parser, T_FOR);
            struct Node *iter = parse_iterate(parser);

            struct Node *cond = NULL;
            if (curtok(parser) == T_IF) {
                eattok(parser, T_IF);
                cond = parse_expr(parser);
            }

            eattok(parser, T_RBRC);
            struct Node *table_comp = new_TableComp(keys, iter, cond, parser->lex.line);
            return table_comp;
        }
        while (curtok(parser) == T_COMMA) {
            eattok(parser, T_COMMA);
            body_append(&keys, parse_expr(parser));
            eattok(parser, T_COLON);
            body_append(&keys, parse_expr(parser));
        }
        eattok(parser, T_RBRC);
        return new_Table(keys, parser->lex.line);
}


// parse list and table literals
static struct Node *parse_collection(Parser *const parser) {
    eattok(parser, T_LSQB);
    struct Node *keys = new_Body(parser->lex.line);

    // empty list
    if (curtok(parser) == T_RSQB) {
        YASL_TRACE_LOG("%s\n", "Parsing list");
        eattok(parser, T_RSQB);
        return new_List(keys, parser->lex.line);
    }

    body_append(&keys, parse_expr(parser));

    // non-empty list
    if (curtok(parser) == T_FOR) {
        eattok(parser, T_FOR);
        struct Node *iter = parse_iterate(parser);

        struct Node *cond = NULL;
        if (curtok(parser) == T_IF) {
            eattok(parser, T_IF);
            cond = parse_expr(parser);
        }

        eattok(parser, T_RSQB);
        struct Node *table_comp = new_ListComp(keys->children[0], iter, cond, parser->lex.line);
        free(keys);
        return table_comp;
    } else {
        while (curtok(parser) == T_COMMA) {
            YASL_TRACE_LOG("%s\n", "Parsing list");
            eattok(parser, T_COMMA);
            body_append(&keys, parse_expr(parser));
        }
        eattok(parser, T_RSQB);
        return new_List(keys, parser->lex.line);
    }
}
