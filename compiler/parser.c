#include "parser.h"

#include <inttypes.h>

#include "ast.h"
#include "debug.h"
#include "middleend.h"
#include "yasl_conf.h"
#include "yasl_error.h"
#include "yasl_include.h"


static struct Node *parse_program(struct Parser *const parser);
static struct Node *parse_const(struct Parser *const parser);
static struct Node *parse_let(struct Parser *const parser);
static struct Node *parse_decl(struct Parser *const parser);
static struct Node *parse_fn(struct Parser *const parser);
static struct Node *parse_for(struct Parser *const parser);
static struct Node *parse_while(struct Parser *const parser);
static struct Node *parse_if(struct Parser *const parser);
static struct Node *parse_expr(struct Parser *const parser);
static struct Node *parse_assign(struct Parser *const parser, struct Node *cur_node);
static struct Node *parse_ternary(struct Parser *const parser);
static struct Node *parse_undef_or(struct Parser *const parser);
static struct Node *parse_or(struct Parser *const parser);
static struct Node *parse_and(struct Parser *const parser);
static struct Node *parse_bor(struct Parser *const parser);
static struct Node *parse_bxor(struct Parser *const parser);
static struct Node *parse_band(struct Parser *const parser);
static struct Node *parse_equals(struct Parser *const parser);
static struct Node *parse_comparator(struct Parser *const parser);
static struct Node *parse_concat(struct Parser *const parser);
static struct Node *parse_bshift(struct Parser *const parser);
static struct Node *parse_add(struct Parser *const parser);
static struct Node *parse_multiply(struct Parser *const parser);
static struct Node *parse_unary(struct Parser *const parser);
static struct Node *parse_power(struct Parser *const parser);
static struct Node *parse_call(struct Parser *const parser);
static struct Node *parse_constant(struct Parser *const parser);
static struct Node *parse_id(struct Parser *const parser);
static struct Node *parse_undef(struct Parser *const parser);
static struct Node *parse_float(struct Parser *const parser);
static struct Node *parse_integer(struct Parser *const parser);
static struct Node *parse_boolean(struct Parser *const parser);
static struct Node *parse_string(struct Parser *const parser);
static struct Node *parse_table(struct Parser *const parser);
static struct Node *parse_lambda(struct Parser *const parser);
static struct Node *parse_collection(struct Parser *const parser);

#define parser_print_err(parser, format, ...) {\
	char *tmp = (char *)malloc(snprintf(NULL, 0, format, __VA_ARGS__) + 1);\
	sprintf(tmp, format, __VA_ARGS__);\
	(parser)->lex.err.print(&(parser)->lex.err, tmp, strlen(tmp));\
	free(tmp);\
}

#define parser_print_err_syntax(parser, format, ...) parser_print_err(parser, "SyntaxError: " format, __VA_ARGS__)

int peof(const struct Parser *const parser) {
	return parser->lex.type == T_EOF;
}

//NOTE: keep this updated alongside lexer.h
static inline int tok_isaugmented(const enum Token t) {
	// ^=, *=, /=, //=,
	// %=, +=, -=, >>=, <<=,
	// ||=, &&=, ~=, **=, |=,
	// ??=
	return t == T_CARETEQ || t == T_STAREQ || t == T_SLASHEQ || t == T_DSLASHEQ ||
	       t == T_MODEQ || t == T_PLUSEQ || t == T_MINUSEQ || t == T_DGTEQ || t == T_DLTEQ ||
	       t == T_DBAREQ || t == T_DAMPEQ || t == T_TILDEEQ || t == T_AMPEQ || t == T_AMPCARETEQ ||
	       t == T_DSTAREQ || t == T_BAREQ ||
	       t == T_DQMARKEQ;
}

static inline enum Token curtok(const struct Parser *const parser) {
	return parser->lex.type;
}

void parser_cleanup(struct Parser *const parser) {
	lex_cleanup(&parser->lex);
}

static struct Node *handle_error(struct Parser *const parser) {
	parser->status = YASL_SYNTAX_ERROR;
	free(parser->lex.value);
	parser->lex.value = NULL;

	while (parser->lex.c != '\n' && !lxeof(parser->lex.file)) {
		lex_getchar(&parser->lex);
	}
	return NULL;
}

enum Token eattok(struct Parser *const parser, const enum Token token) {
	if (curtok(parser) != token) {
		if (curtok(parser) == T_UNKNOWN) {
			parser->status = parser->lex.status;
		} else {
			YASL_PRINT_ERROR_SYNTAX("Expected %s, got %s, in line %" PRI_SIZET "\n", YASL_TOKEN_NAMES[token],
						YASL_TOKEN_NAMES[curtok(parser)], parser->lex.line);
			parser->status = YASL_SYNTAX_ERROR;
		}
		while (!TOKEN_MATCHES(parser, T_SEMI, T_EOF)) {
			free(parser->lex.value);
			gettok(&parser->lex);
		}
	} else {
		gettok(&parser->lex);
	}
	return token;
}

static char *eatname(struct Parser *const parser) {
	char *tmp = parser->lex.value;
	eattok(parser, T_ID);
	return tmp;
}

bool matcheattok(struct Parser *const parser, const enum Token token) {
	if (TOKEN_MATCHES(parser, token)) {
		eattok(parser, token);
		return true;
	}
	return false;
}

struct Node *parse(struct Parser *const parser) {
	return parse_program(parser);
}

struct Node *parse_assign_or_exprstmt(struct Parser *const parser) {
	size_t line = parser->lex.line;
	struct Node *expr = parse_expr(parser);

	if (curtok(parser) == T_EQ || tok_isaugmented(curtok(parser))) {
		return parse_assign(parser, expr);
	}

	return new_ExprStmt(expr, line);
}
static bool isfndecl(struct Parser *const parser) {
	(void) parser;
	long curr = lxtell(parser->lex.file);
	eattok(parser, T_FN);
	free(parser->lex.value);
	if (matcheattok(parser, T_ID)) {
		lxseek(parser->lex.file, curr, SEEK_SET);
		parser->lex.type = T_FN;
		return true;
	} else {
		lxseek(parser->lex.file, curr, SEEK_SET);
		parser->lex.type = T_FN;
		return false;
	}
}

static struct Node *parse_program(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing statement in line %" PRI_SIZET "\n", parser->lex.line);
	size_t line;
	switch (curtok(parser)) {
	case T_ECHO:
		eattok(parser, T_ECHO);
		return new_Print(parse_expr(parser), parser->lex.line);
	case T_FN:
		if (isfndecl(parser)) return parse_fn(parser);
		else return parse_expr(parser);
	case T_RET:
		eattok(parser, T_RET);
		return new_Return(parse_expr(parser), parser->lex.line);
	case T_EXPORT:
		eattok(parser, T_EXPORT);
		return new_Export(parse_expr(parser), parser->lex.line);
	case T_CONST:
		return parse_const(parser);
	case T_LET:
		return parse_decl(parser);
	case T_FOR:
		return parse_for(parser);
	case T_WHILE:
		return parse_while(parser);
	case T_BREAK:
		line = parser->lex.line;
		eattok(parser, T_BREAK);
		return new_Break(line);
	case T_CONT:
		line = parser->lex.line;
		eattok(parser, T_CONT);
		return new_Continue(line);
	case T_IF:
		return parse_if(parser);
	case T_ELSEIF:
	case T_ELSE:
		parser_print_err_syntax(parser,
			"`%s` without previous `if` (line %" PRI_SIZET ").\n",
			YASL_TOKEN_NAMES[curtok(parser)],
			parser->lex.line);
		return handle_error(parser);
	case T_UNKNOWN:
		parser->status = parser->lex.status;
		return NULL;
	default:
		return parse_assign_or_exprstmt(parser);
	}
}

static struct Node *parse_body(struct Parser *const parser) {
	eattok(parser, T_LBRC);
	struct Node *body = new_Body(parser->lex.line);
	while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
		body_append(&body, parse_program(parser));
		eattok(parser, T_SEMI);
	}
	eattok(parser, T_RBRC);
	return body;
}

static struct Node *parse_function_params(struct Parser *const parser) {
	struct Node *block = new_Body(parser->lex.line);
	while (TOKEN_MATCHES(parser, T_ID, T_CONST)) {
		if (TOKEN_MATCHES(parser, T_ID)) {
			struct Node *cur_node = parse_id(parser);
			cur_node->nodetype = N_LET;
			body_append(&block, cur_node);
		} else {
			eattok(parser, T_CONST);
			struct Node *cur_node = parse_id(parser);
			cur_node->nodetype = N_CONST;
			body_append(&block, cur_node);
		}
		if (!matcheattok(parser, T_COMMA)) break;
	}
	return block;
}

static struct Node *parse_fn(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing fn in line %" PRI_SIZET "\n", parser->lex.line);
	eattok(parser, T_FN);
	size_t line = parser->lex.line;
	char *name = eatname(parser);
	size_t name_len = strlen(name);
	if (matcheattok(parser, T_DOT)) {
		struct Node *collection = new_Var(name, line);
		size_t line = parser->lex.line;
		char *name = eatname(parser);
		size_t name_len = strlen(name);

		struct Node *index = new_String(name, name_len, line);

		eattok(parser, T_LPAR);
		struct Node *block = parse_function_params(parser);
		eattok(parser, T_RPAR);

		struct Node *body = parse_body(parser);

		return new_Set(collection, index, new_FnDecl(block, body, NULL, 0, parser->lex.line), line);
	}
	eattok(parser, T_LPAR);
	struct Node *block = parse_function_params(parser);
	eattok(parser, T_RPAR);

	struct Node *body = parse_body(parser);

	char *name2 = (char *)malloc(name_len + 1);
	strcpy(name2, name);
	return new_Let(name, new_FnDecl(block, body, name2, strlen(name2), parser->lex.line), line);
	// TODO Fix this ^
}

static struct Node *parse_const(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing const in line %" PRI_SIZET "\n", parser->lex.line);
	eattok(parser, T_CONST);
	if (matcheattok(parser, T_FN)) {
		size_t line = parser->lex.line;
		char *name = eatname(parser);
		size_t name_len = strlen(name);
		eattok(parser, T_LPAR);
		struct Node *block = parse_function_params(parser);
		eattok(parser, T_RPAR);

		struct Node *body = parse_body(parser);

		// TODO: clean this up
		char *name2 = (char *)malloc(name_len + 1);
		strcpy(name2, name);
		return new_Const(name, new_FnDecl(block, body, name2, strlen(name2), parser->lex.line), line);
	}
	size_t line = parser->lex.line;
	char *name = eatname(parser);
	eattok(parser, T_EQ);
	struct Node *expr = parse_expr(parser);
	return new_Const(name, expr, line);
}

static struct Node *parse_decl(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing let in line %" PRI_SIZET "\n", parser->lex.line);
	size_t i = 0;
	struct Node *buffer = new_Body(parser->lex.line);

	do {
		eattok(parser, T_LET);
		size_t line = parser->lex.line;
		char *name = eatname(parser);
		body_append(&buffer, new_Let(name, NULL, line));
		i++;
	} while (matcheattok(parser, T_COMMA));

	eattok(parser, T_EQ);

	size_t j = 0;
	do {
		buffer->children[j]->children[0] = parse_expr(parser);
	} while (j++ < i && matcheattok(parser, T_COMMA));

	buffer->nodetype = N_DECL;
	return buffer;
}

static struct Node *parse_let(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing let in line %" PRI_SIZET "\n", parser->lex.line);
	eattok(parser, T_LET);
	size_t line = parser->lex.line;
	char *name = eatname(parser);
	eattok(parser, T_EQ);
	struct Node *expr = parse_expr(parser);
	return new_Let(name, expr, line);
}

static struct Node *parse_iterate(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing let <- in line %" PRI_SIZET "\n", parser->lex.line);
	size_t line = parser->lex.line;
	char *name = eatname(parser);
	eattok(parser, T_LEFT_ARR);
	struct Node *collection = parse_expr(parser);
	return new_LetIter(name, collection, line);
}

static struct Node *parse_let_iterate_or_let(struct Parser *const parser) {
	if (curtok(parser) == T_LET) {
		return parse_let(parser);
	} else {
		return parse_iterate(parser);
	}
}

static struct Node *parse_for(struct Parser *const parser) {
	eattok(parser, T_FOR);

	struct Node *iter = parse_let_iterate_or_let(parser);

	if (iter->nodetype == N_LETITER) {
		struct Node *body = parse_body(parser);
		return new_ForIter(iter, body, parser->lex.line);
	} else {
		eattok(parser, T_SEMI);
		struct Node *cond = parse_expr(parser);
		eattok(parser, T_SEMI);
		struct Node *post = parse_assign_or_exprstmt(parser);
		struct Node *body = parse_body(parser);
		struct Node *outer_body = new_Body(parser->lex.line);
		body_append(&outer_body, iter);
		body_append(&outer_body, new_While(cond, body, new_ExprStmt(post, parser->lex.line), parser->lex.line));
		struct Node *block = new_Block(outer_body, parser->lex.line);
		return block;
	}
}

static struct Node *parse_while(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing while in line %" PRI_SIZET "\n", parser->lex.line);
	eattok(parser, T_WHILE);
	struct Node *cond = parse_expr(parser);
	struct Node *body = parse_body(parser);
	return new_While(cond, new_Block(body, parser->lex.line), NULL, parser->lex.line);
}

static struct Node *parse_if(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing if in line %" PRI_SIZET "\n", parser->lex.line);
	if (matcheattok(parser, T_IF)) ;
	else if (matcheattok(parser, T_ELSEIF)) ;
	else {
		YASL_PRINT_ERROR_SYNTAX("Expected if or elseif, got %s\n", YASL_TOKEN_NAMES[curtok(parser)]);
		return handle_error(parser);
	}
	struct Node *cond = parse_expr(parser);
	struct Node *then_block = parse_body(parser);
	if (curtok(parser) != T_ELSE && curtok(parser) != T_ELSEIF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "no else");
		return new_If(cond, new_Block(then_block, parser->lex.line), NULL, parser->lex.line);
	}
	if (curtok(parser) == T_ELSEIF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "elseif");
		return new_If(cond, new_Block(then_block, parser->lex.line), parse_if(parser), parser->lex.line);
	}
	if (matcheattok(parser, T_ELSE)) {
		YASL_PARSE_DEBUG_LOG("%s\n", "else");
		struct Node *else_block = parse_body(parser);
		return new_If(cond, new_Block(then_block, parser->lex.line), new_Block(else_block, parser->lex.line), parser->lex.line);
	}
	YASL_PRINT_ERROR_SYNTAX("Expected newline, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
	return handle_error(parser);
}

static struct Node *parse_expr(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "parsing expression.");
	struct Node *node = parse_ternary(parser);
	if (node)
		fold(node);
	return node;
}

static struct Node *parse_assign(struct Parser *const parser, struct Node *cur_node) {
	YASL_PARSE_DEBUG_LOG("parsing = in line %" PRI_SIZET "\n", parser->lex.line);
	size_t line = parser->lex.line;
	if (matcheattok(parser, T_EQ)) {
		switch (cur_node->nodetype) {
		case N_VAR: {
			struct Node *assign_node = new_Assign(cur_node->value.sval.str, parse_expr(parser), line);
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
			parser_print_err_syntax(parser, "Invalid l-value (line %" PRI_SIZET ").\n", line);
			return handle_error(parser);
		}
	} else if (tok_isaugmented(curtok(parser))) {
	  enum Token op = (enum Token)(eattok(parser, curtok(parser)) - 1); // relies on enum in lexer.h
		switch (cur_node->nodetype) {
		case N_VAR: {
			char *name = cur_node->value.sval.str;
			struct Node *tmp = node_clone(cur_node);
			free(cur_node);
			return new_Assign(name, new_BinOp(op, tmp, parse_expr(parser), line), line);
		}
		case N_GET: {
			struct Node *collection = cur_node->children[0];
			struct Node *key = cur_node->children[1];
			struct Node *value = new_BinOp(op, node_clone(cur_node), parse_expr(parser), line);
			free(cur_node);
			return new_Set(collection, key, value, line);
		}
		default:
			parser_print_err_syntax(parser, "Invalid l-value (line %" PRI_SIZET ").\n", line);
			return handle_error(parser);
		}
	}
	return cur_node;
}

static struct Node *parse_ternary(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing ?: in line %" PRI_SIZET "\n", parser->lex.line);
	struct Node *cur_node = parse_undef_or(parser);
	if (matcheattok(parser, T_QMARK)) {
		struct Node *left = parse_ternary(parser);
		eattok(parser, T_COLON);
		struct Node *right = parse_ternary(parser);
		return new_TriOp(T_QMARK, cur_node, left, right, parser->lex.line);
	}
	return cur_node;
}


#define BINOP_R(name, next, msg, ...)\
static struct Node *parse_##name(struct Parser *const parser) {\
        YASL_PARSE_DEBUG_LOG("parsing " msg " in line %" PRI_SIZET "\n", parser->lex.line);\
        struct Node *cur_node = parse_##next(parser);\
        if (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
                enum Token op = eattok(parser, curtok(parser));\
                return new_BinOp(op, cur_node, parse_##name(parser), parser->lex.line);\
        }\
        return cur_node;\
}

#define BINOP_L(name, next, msg, ...) \
static struct Node *parse_##name(struct Parser *const parser) {\
        YASL_PARSE_DEBUG_LOG("parsing " msg " in line %" PRI_SIZET "\n", parser->lex.line);\
        struct Node *cur_node = parse_##next(parser);\
        while (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
                enum Token op = eattok(parser, curtok(parser));\
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

static struct Node *parse_unary(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing !, -, +, ^, len in line %" PRI_SIZET "\n", parser->lex.line);
	if (curtok(parser) == T_PLUS || curtok(parser) == T_MINUS || curtok(parser) == T_BANG ||
	    curtok(parser) == T_CARET || curtok(parser) == T_LEN) {
		enum Token op = eattok(parser, curtok(parser));
		return new_UnOp(op, parse_unary(parser), parser->lex.line);
	} else {
		return parse_power(parser);
	}
}

static struct Node *parse_power(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing ** in line %" PRI_SIZET "\n", parser->lex.line);
	struct Node *cur_node = parse_call(parser);
	if (matcheattok(parser, T_DSTAR)) {
		return new_BinOp(T_DSTAR, cur_node, parse_unary(parser), parser->lex.line);
	}
	return cur_node;
}

static struct Node *parse_call(struct Parser *const parser) {
	struct Node *cur_node = parse_constant(parser);
	while (TOKEN_MATCHES(parser, T_LSQB, T_DOT, T_LPAR, T_RIGHT_ARR)) {
		if (matcheattok(parser, T_RIGHT_ARR)) {
			struct Node *right = parse_constant(parser);
			if (right->nodetype != N_VAR) {
				parser_print_err_syntax(parser, "Invalid method call (line %" PRI_SIZET ").\n", parser->lex.line);
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

		} else if (matcheattok(parser, T_DOT)) {
			struct Node *right = parse_constant(parser);
			if (right->nodetype == N_CALL) {
				cur_node = new_Set(cur_node, right->children[0]->children[0],
						   right->children[0]->children[1], parser->lex.line);
				free(right);
			} else if (right->nodetype == N_VAR) {
				right->nodetype = N_STR;
				cur_node = new_Get(cur_node, right, parser->lex.line);
			} else {
				parser_print_err_syntax(parser, "Invalid member access (line %" PRI_SIZET ").\n", parser->lex.line);
				return handle_error(parser);
			}
		} else if (matcheattok(parser, T_LSQB)) {
			size_t line = parser->lex.line;
			struct Node *expr = parse_expr(parser);
			if (matcheattok(parser, T_COLON)) {
				struct Node *end = parse_expr(parser);
				cur_node = new_Slice(cur_node, expr, end, line);
			} else {
				cur_node = new_Get(cur_node, expr, line);
			}
			eattok(parser, T_RSQB);
		} else if (matcheattok(parser, T_LPAR)) {
			YASL_PARSE_DEBUG_LOG("%s\n", "Parsing function call");
			cur_node = new_Call(new_Body(parser->lex.line), cur_node, parser->lex.line);
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

static struct Node *parse_constant(struct Parser *const parser) {
	switch (curtok(parser)) {
	case T_DOT:eattok(parser, T_DOT); {
		struct Node *cur_node = new_String(parser->lex.value, strlen(parser->lex.value), parser->lex.line);
		eattok(parser, T_ID);
		return cur_node;
	  }
	case T_ID: return parse_id(parser);
	case T_LPAR:eattok(parser, T_LPAR); {
		struct Node *expr = parse_expr(parser);
		eattok(parser, T_RPAR);
		return expr;
	  }
	case T_LSQB: return parse_collection(parser);
	case T_LBRC: return parse_table(parser);
	case T_STR: return parse_string(parser);
	case T_INT: return parse_integer(parser);
	case T_FLOAT: return parse_float(parser);
	case T_BOOL: return parse_boolean(parser);
	case T_UNDEF: return parse_undef(parser);
	case T_FN: return parse_lambda(parser);
		// handle invalid expressions with sensible error messages.
	case T_ECHO:
	case T_WHILE:
	case T_BREAK:
	case T_RET:
	case T_CONT:
	case T_IF:
	case T_ELSEIF:
	case T_ELSE:
		parser_print_err_syntax(parser, "Expected expression, got `%s` (line %" PRI_SIZET ").\n",
					    YASL_TOKEN_NAMES[curtok(parser)], parser->lex.line);
		return handle_error(parser);
	case T_UNKNOWN:
		parser->status = parser->lex.status;
		return NULL;
	default:
		YASL_PRINT_ERROR_SYNTAX("Invalid expression in line %" PRI_SIZET " (%s).\n", parser->lex.line, YASL_TOKEN_NAMES[curtok(parser)]);
		return handle_error(parser);
	}
}

static struct Node *parse_id(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing variable");
	size_t line = parser->lex.line;
	char *name = eatname(parser);
	struct Node *cur_node = new_Var(name, line);
	return cur_node;
}

static struct Node *parse_undef(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing undef");
	struct Node *cur_node = new_Undef(parser->lex.line);
	eattok(parser, T_UNDEF);
	return cur_node;
}

static struct Node *parse_lambda(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing lambda");
	size_t line = parser->lex.line;

	eattok(parser, T_FN);
	eattok(parser, T_LPAR);
	struct Node *block = parse_function_params(parser);
	eattok(parser, T_RPAR);
	struct Node *body = parse_body(parser);

	return new_FnDecl(block, body, NULL, 0, line);
}


static yasl_float get_float(char *buffer) {
	return strtod(buffer, (char **) NULL);
}

static struct Node *parse_float(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing float");
	struct Node *cur_node = new_Float(get_float(parser->lex.value), parser->lex.line);
	free(parser->lex.value);
	eattok(parser, T_FLOAT);
	return cur_node;
}

static yasl_int get_int(char *buffer) {
	if (strlen(buffer) < 2) {
		return strtoll(buffer, (char **) NULL, 10);
	}
	switch (buffer[1]) {
	case 'x': return strtoll(buffer + 2, (char **) NULL, 16);
	case 'b': return strtoll(buffer + 2, (char **) NULL, 2);
	default: return strtoll(buffer, (char **) NULL, 10);
	}
}

static struct Node *parse_integer(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing int");
	struct Node *cur_node = new_Integer(get_int(parser->lex.value), parser->lex.line);
	free(parser->lex.value);
	eattok(parser, T_INT);
	return cur_node;
}

static struct Node *parse_boolean(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing bool");
	struct Node *cur_node = new_Boolean(!strcmp(parser->lex.value, "true"), parser->lex.line);
	free(parser->lex.value);
	eattok(parser, T_BOOL);
	return cur_node;
}

static struct Node *parse_string(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing str");
	struct Node *cur_node = new_String(parser->lex.value, parser->lex.val_len, parser->lex.line);

	// interpolated strings
	while (parser->lex.mode == L_INTERP) {
		eattok(parser, T_STR);
		eattok(parser, T_LBRC);
		parser->lex.mode = L_NORMAL;
		struct Node *expr = parse_expr(parser);
		parser->lex.mode = L_INTERP;
		cur_node = new_BinOp(T_TILDE, cur_node, expr, parser->lex.line);
		if (parser->lex.c == '}') {
			parser->lex.c = lxgetc(parser->lex.file);
		} else {
			parser_print_err_syntax(parser, "Expected } in line %" PRI_SIZET ".\n", parser->lex.line);
			node_del(cur_node);
			return handle_error(parser);
		}
		lex_eatinterpstringbody(&parser->lex);
		if (parser->lex.status) {
			node_del(cur_node);
			return handle_error(parser);
		};
		struct Node *str = new_String(parser->lex.value, parser->lex.val_len, parser->lex.line);
		cur_node = new_BinOp(T_TILDE, cur_node, str, parser->lex.line);
	}

	eattok(parser, T_STR);

	return cur_node;
}


static struct Node *parse_table(struct Parser *const parser) {
	eattok(parser, T_LBRC);
	struct Node *keys = new_Body(parser->lex.line);

	// empty table
	if (matcheattok(parser, T_RBRC)) {
		YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table");
		return new_Table(keys, parser->lex.line);
	}

	body_append(&keys, parse_expr(parser));

	// non-empty table
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table");
	eattok(parser, T_COLON);
	body_append(&keys, parse_expr(parser));

	if (matcheattok(parser, T_FOR)) {
		struct Node *iter = parse_iterate(parser);

		struct Node *cond = NULL;
		if (matcheattok(parser, T_IF)) {
			cond = parse_expr(parser);
		}

		eattok(parser, T_RBRC);
		struct Node *table_comp = new_TableComp(keys, iter, cond, parser->lex.line);
		return table_comp;
	}
	while (matcheattok(parser, T_COMMA)) {
		body_append(&keys, parse_expr(parser));
		eattok(parser, T_COLON);
		body_append(&keys, parse_expr(parser));
	}
	eattok(parser, T_RBRC);
	return new_Table(keys, parser->lex.line);
}


// parse list and table literals
static struct Node *parse_collection(struct Parser *const parser) {
	eattok(parser, T_LSQB);
	struct Node *keys = new_Body(parser->lex.line);

	// empty list
	if (matcheattok(parser, T_RSQB)) {
		YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list");
		return new_List(keys, parser->lex.line);
	}

	body_append(&keys, parse_expr(parser));

	// non-empty list
	if (matcheattok(parser, T_FOR)) {
		struct Node *iter = parse_iterate(parser);

		struct Node *cond = NULL;
		if (matcheattok(parser, T_IF)) {
			cond = parse_expr(parser);
		}

		eattok(parser, T_RSQB);
		struct Node *table_comp = new_ListComp(keys->children[0], iter, cond, parser->lex.line);
		free(keys);
		return table_comp;
	} else {
		while (matcheattok(parser, T_COMMA)) {
			YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list");
			body_append(&keys, parse_expr(parser));
		}
		eattok(parser, T_RSQB);
		return new_List(keys, parser->lex.line);
	}
}

#ifdef _MSC_VER
// To avoid MSVC _VA_ARGS_ macro expansion bug
int token_matches(struct Parser *const parser, ...) {
    va_list ap;
    int ret = 0;
    va_start(ap, parser);
    for (;;) {
        enum Token tok = va_arg(ap, enum Token);
        if (tok == -1) break;
        if (curtok(parser) == tok) {
            ret = 1;
            break;
 	}
    }
    va_end(ap);
    return ret;
}
#endif
