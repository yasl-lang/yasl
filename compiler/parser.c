#include "parser.h"

#include <inttypes.h>
#include <stdarg.h>

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
static struct Node *parse_return(struct Parser *const parser);
static struct Node *parse_for(struct Parser *const parser);
static struct Node *parse_while(struct Parser *const parser);
static struct Node *parse_match(struct Parser *const parser);
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
static struct Node *parse_list(struct Parser *const parser);
static struct Node *parse_assert(struct Parser *const parser);

YASL_FORMAT_CHECK static void parser_print_err(struct Parser *parser, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	parser->lex.err.print(&parser->lex.err, fmt, args);
	va_end(args);
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
			parser_print_err_syntax(parser, "Expected %s, got %s (line %" PRI_SIZET ").\n", YASL_TOKEN_NAMES[token],
						YASL_TOKEN_NAMES[curtok(parser)], parser->lex.line);
			parser->status = YASL_SYNTAX_ERROR;
		}
		while (!TOKEN_MATCHES(parser, T_SEMI, T_EOF)) {
			free(parser->lex.value);
			gettok(&parser->lex);
		}
		return T_UNKNOWN;
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

static struct Node *parse_decl_helper(struct Parser *const parser, struct Node *buffer, size_t i);

struct Node *parse_assign_or_exprstmt(struct Parser *const parser) {
	size_t line = parser->lex.line;
	struct Node *expr = parse_expr(parser);

	if (curtok(parser) == T_EQ || tok_isaugmented(curtok(parser))) {
		return parse_assign(parser, expr);
	}

	if (curtok(parser) == T_COMMA && expr->nodetype == N_GET) {
		struct Node *buffer = new_Body(parser->lex.line);

		struct Node *set = new_Set(Get_get_collection(expr), Get_get_value(expr), NULL, line);
		free(expr);
		body_append(&buffer, set);

		return parse_decl_helper(parser, buffer, 0);

	}

	return new_ExprStmt(expr, line);
}

/*
 * Checks for function statement `fn <id> ...` vs function expr `fn ( ...`.
 */
static bool isfndecl(struct Parser *const parser) {
	long curr = lxtell(parser->lex.file);
	eattok(parser, T_FN);
	free(parser->lex.value);
	bool result = TOKEN_MATCHES(parser, T_ID);
	lxseek(parser->lex.file, curr, SEEK_SET);
	parser->lex.type = T_FN;
	return result;
}

/*
 * Checks for const function statement `const fn ...` vs const decl `const <id> ...`.
 */
static bool isconstfndecl(struct Parser *const parser) {
	long curr = lxtell(parser->lex.file);
	eattok(parser, T_CONST);
	free(parser->lex.value);
	bool result = TOKEN_MATCHES(parser, T_FN);
	lxseek(parser->lex.file, curr, SEEK_SET);
	parser->lex.type = T_CONST;
	return result;
}

/*
 * Checks for multiple assignment `<id>, ...` vs other uses of identifiers.
 */
static bool ismultiassign(struct Parser *const parser) {
	long curr = lxtell(parser->lex.file);
	char *name = parser->lex.value;
	parser->lex.value = NULL;
	eattok(parser, T_ID);
	bool result = TOKEN_MATCHES(parser, T_COMMA);
	lxseek(parser->lex.file, curr, SEEK_SET);
	free(parser->lex.value);
	parser->lex.type = T_ID;
	parser->lex.value = name;
	return result;
}

static struct Node *parse_program(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing statement in line %" PRI_SIZET "\n", parser->lex.line);
	size_t line = parser->lex.line;
	switch (curtok(parser)) {
	case T_ECHO:
		eattok(parser, T_ECHO);
		return new_Print(parse_expr(parser), line);
	case T_FN:
		if (isfndecl(parser)) return parse_fn(parser);
		else return parse_expr(parser);
	case T_RET:
		return parse_return(parser);
	case T_EXPORT:
		eattok(parser, T_EXPORT);
		return new_Export(parse_expr(parser), line);
	case T_CONST:
		if (isconstfndecl(parser)) return parse_const(parser);
		else return parse_decl(parser);
	case T_LET:
		return parse_decl(parser);
	case T_ID:
		if (ismultiassign(parser)) return parse_decl(parser);
		else return parse_assign_or_exprstmt(parser);
	case T_FOR:
		return parse_for(parser);
	case T_WHILE:
		return parse_while(parser);
	case T_BREAK:
		eattok(parser, T_BREAK);
		return new_Break(line);
	case T_CONT:
		eattok(parser, T_CONT);
		return new_Continue(line);
	case T_MATCH:
		return parse_match(parser);
	case T_IF:
		return parse_if(parser);
	case T_ELSEIF:
	case T_ELSE:
		parser_print_err_syntax(parser,
			"`%s` without previous `if` (line %" PRI_SIZET ").\n",
			YASL_TOKEN_NAMES[curtok(parser)],
			line);
		return handle_error(parser);
	case T_ASS:
		return parse_assert(parser);
	case T_UNKNOWN:
		parser->status = parser->lex.status;
		return NULL;
	default:
		return parse_assign_or_exprstmt(parser);
	}
}

static struct Node *parse_body(struct Parser *const parser) {
	size_t line = parser->lex.line;
	eattok(parser, T_LBRC);
	struct Node *body = new_Body(line);
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

static struct Node *parse_return(struct Parser *const parser) {
	size_t line = parser->lex.line;
	eattok(parser, T_RET);
	struct Node *expr = parse_expr(parser);
	/*
	if (TOKEN_MATCHES(parser, T_COMMA)) {
		struct Node *block = new_Body(line);
		body_append(&block, expr);
		while (matcheattok(parser, T_COMMA)) {
			expr = parse_expr(parser);
			body_append(&block, expr);
		}
		return new_MultiReturn(block, line);
	}
	 */
	return new_Return(expr, line);
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

		return new_Set(collection, index, new_FnDecl(block, body, NULL, 0, line), line);
	}
	eattok(parser, T_LPAR);
	struct Node *block = parse_function_params(parser);
	eattok(parser, T_RPAR);

	struct Node *body = parse_body(parser);

	char *name2 = (char *)malloc(name_len + 1);
	strcpy(name2, name);
	return new_Let(name, new_FnDecl(block, body, name2, strlen(name2), line), line);
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
		return new_Const(name, new_FnDecl(block, body, name2, strlen(name2), line), line);
	}
	size_t line = parser->lex.line;
	char *name = eatname(parser);
	eattok(parser, T_EQ);
	struct Node *expr = parse_expr(parser);
	return new_Const(name, expr, line);
}

static struct Node *parse_let_const_or_var(struct Parser *const parser) {
	size_t line = parser->lex.line;
	if (matcheattok(parser, T_LET)) {
		char *name = eatname(parser);
		return new_Let(name, NULL, line);
	} else if (matcheattok(parser, T_CONST)) {
		char *name = eatname(parser);
		return new_Const(name, NULL, line);
	} else {
		struct Node *node = parse_call(parser);
		if (node->nodetype == N_VAR) {
			struct Node *assign = new_Assign(node->value.sval.str, NULL, line);
			free(node);
			return assign;
		} else if (node->nodetype == N_GET) {
			struct Node *set = new_Set(Get_get_collection(node), Get_get_value(node), NULL, line);
			free(node);
			return set;
		} else {
			parser_print_err_syntax(parser, "Expected `let`, `const`, or id, got %s", YASL_TOKEN_NAMES[curtok(parser)]);
			return handle_error(parser);
		}
	}
}

static struct Node *parse_decl_helper(struct Parser *const parser, struct Node *buffer, size_t i) {
	while (matcheattok(parser, T_COMMA)) {
		struct Node *lval = parse_let_const_or_var(parser);
		body_append(&buffer, lval);
		i++;
	}

	eattok(parser, T_EQ);

	size_t j = 0;
	do {
		struct Node *child = buffer->children[j];
		if (child->nodetype == N_SET) {
			child->children[2] = parse_expr(parser);
		} else {
			child->children[0] = parse_expr(parser);
		}
	} while (j++ < i && matcheattok(parser, T_COMMA));

	buffer->nodetype = N_DECL;
	return buffer;
}

static struct Node *parse_decl(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing let in line %" PRI_SIZET "\n", parser->lex.line);
	size_t i = 0;
	struct Node *buffer = new_Body(parser->lex.line);

	struct Node *lval = parse_let_const_or_var(parser);
	body_append(&buffer, lval);
	i++;

	return parse_decl_helper(parser, buffer, i);
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
	size_t line = parser->lex.line;
	eattok(parser, T_FOR);

	struct Node *iter = parse_let_iterate_or_let(parser);

	if (iter->nodetype == N_LETITER) {
		struct Node *body = parse_body(parser);
		return new_ForIter(iter, body, line);
	} else {
		eattok(parser, T_SEMI);
		struct Node *cond = parse_expr(parser);
		eattok(parser, T_SEMI);
		struct Node *post = parse_assign_or_exprstmt(parser);
		struct Node *body = parse_body(parser);
		struct Node *outer_body = new_Body(line);
		body_append(&outer_body, iter);
		body_append(&outer_body, new_While(cond, new_Block(body, line), new_ExprStmt(post, line), line));
		struct Node *block = new_Block(outer_body, line);
		return block;
	}
}

static struct Node *parse_while(struct Parser *const parser) {
	size_t line = parser->lex.line;
	YASL_PARSE_DEBUG_LOG("parsing while in line %" PRI_SIZET "\n", line);
	eattok(parser, T_WHILE);
	struct Node *cond = parse_expr(parser);
	struct Node *body = parse_body(parser);
	return new_While(cond, new_Block(body, line), NULL, line);
}

static struct Node *parse_pattern(struct Parser *const parser);

static struct Node *parse_primitivepattern(struct Parser *const parser) {
	size_t line = parser->lex.line;
	struct Node *n;

	switch (curtok(parser)) {
	case T_INT:
		n = parse_integer(parser);
		n->nodetype = N_PATINT;
		return n;
	case T_FLOAT:
		n = parse_float(parser);
		n->nodetype = N_PATFL;
		return n;
	case T_BOOL:
		n = parse_boolean(parser);
		n->nodetype = N_PATBOOL;
		return n;
	case T_MINUS:
		eattok(parser, T_MINUS);
		if (curtok(parser) == T_INT) {
			n = parse_integer(parser);
			n->nodetype = N_PATINT;
			n->value.ival *= -1;
			return n;
		} else if (curtok(parser) == T_FLOAT) {
			n = parse_float(parser);
			n->nodetype = N_PATFL;
			n->value.dval *= -1;
			return n;
		} else {
			parser_print_err_syntax(parser, "Expected numeric pattern, got pattern starting in %s (line %" PRI_SIZET ").\n", YASL_TOKEN_NAMES[curtok(parser)], line);
			return handle_error(parser);
		}
	case T_STR:
		if (parser->lex.mode == L_INTERP) {
			while (parser->lex.c != '"') {
				lex_getchar(&parser->lex);
			}
			eattok(parser, T_STR);
			parser_print_err_syntax(parser, "Interpolated strings cannot be used in patterns (line %" PRI_SIZET ").\n", line);
			return handle_error(parser);
		}
		n = parse_string(parser);
		n->nodetype = N_PATSTR;
		return n;
	case T_DOT:
		eattok(parser, T_DOT);
		n = parse_id(parser);
		n->nodetype = N_PATSTR;
		return n;
	default:
		parser_print_err_syntax(parser, "Invalid pattern starting in %s (line %" PRI_SIZET ").\n", YASL_TOKEN_NAMES[curtok(parser)], line);
		return handle_error(parser);
	}
}

static struct Node *parse_patternsingle(struct Parser *const parser) {
	size_t line = parser->lex.line;
	struct Node *n;

	switch (curtok(parser)) {
	case T_STAR:
		eattok(parser, T_STAR);
		n = new_Undef(line);
		n->nodetype = N_PATANY;
		return n;
	case T_UNDEF:
		eattok(parser, T_UNDEF);
		n = new_Undef(line);
		n->nodetype = N_PATUNDEF;
		return n;
	case T_LET: {
		eattok(parser, T_LET);
		char *name = eatname(parser);
		n = new_Let(name, NULL, line);
		n->nodetype = N_PATLET;
		return n;
	}
	case T_CONST: {
		eattok(parser, T_CONST);
		char *name = eatname(parser);
		n = new_Const(name, NULL, line);
		n->nodetype = N_PATCONST;
		return n;
	}
	case T_LBRC: {
		eattok(parser, T_LBRC);
		n = new_Body(line);
		n->nodetype = N_PATTABLE;
		if (matcheattok(parser, T_TDOT)) {
			n->nodetype = N_PATVTABLE;
		} else if (curtok(parser) != T_RBRC) {
			body_append(&n, parse_primitivepattern(parser));
			eattok(parser, T_COLON);
			body_append(&n, parse_pattern(parser));
			while (parser->lex.c == '\n') eattok(parser, T_SEMI);
			while (matcheattok(parser, T_COMMA)) {
				YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table pattern");
				if (matcheattok(parser, T_TDOT)) {
					n->nodetype = N_PATVTABLE;
					break;
				}
				body_append(&n, parse_primitivepattern(parser));
				eattok(parser, T_COLON);
				body_append(&n, parse_pattern(parser));
			}
			while (parser->lex.c == '\n') eattok(parser, T_SEMI);
		}
		eattok(parser, T_RBRC);
		return n;
	}
	case T_LSQB:
		eattok(parser, T_LSQB);
		n = new_Body(line);
		n->nodetype = N_PATLS;
		if (matcheattok(parser, T_TDOT)) {
			n->nodetype = N_PATVLS;
		} else if (curtok(parser) != T_RSQB) {
			body_append(&n, parse_pattern(parser));
			while (parser->lex.c == '\n') eattok(parser, T_SEMI);
			while (matcheattok(parser, T_COMMA)) {
				YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list pattern");
				if (matcheattok(parser, T_TDOT)) {
					n->nodetype = N_PATVLS;
					break;
				}
				body_append(&n, parse_pattern(parser));
			}
			while (parser->lex.c == '\n') eattok(parser, T_SEMI);
		}
		eattok(parser, T_RSQB);
		return n;
	case T_LPAR:
		eattok(parser, T_LPAR);
		n = parse_pattern(parser);
		eattok(parser, T_RPAR);
		return n;
	default:
		return parse_primitivepattern(parser);
	}
}

static struct Node *parse_alt(struct Parser *const parser) {
	size_t line = parser->lex.line;
        struct Node *cur_node = parse_patternsingle(parser);
        if (matcheattok(parser, T_BAR)) {
                struct Node *tmp = new_BinOp(T_BAR, cur_node, parse_alt(parser), line);
                tmp->nodetype = N_PATALT;
                return tmp;
        }
        return cur_node;
}

static struct Node *parse_pattern(struct Parser *const parser) {
	return parse_alt(parser);
}

static struct Node *parse_match(struct Parser *const parser) {
	size_t line = parser->lex.line;
	YASL_PARSE_DEBUG_LOG("parsing match in line %" PRI_SIZET "\n", line);
	eattok(parser, T_MATCH);
	struct Node *expr = parse_expr(parser);
	(void)expr;
	eattok(parser, T_LBRC);
	struct Node *pats = new_Body(line);
	struct Node *guards = new_Body(line);
	struct Node *bodies = new_Body(line);
	while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
		body_append(&pats, parse_pattern(parser));
		if (matcheattok(parser, T_IF)) {
			body_append(&guards, parse_expr(parser));
		} else {
			body_append(&guards, NULL);
		}
		body_append(&bodies, parse_body(parser));
		eattok(parser, T_SEMI);
	}
	eattok(parser, T_RBRC);
	return new_Match(expr, pats, guards, bodies, line);
}

static struct Node *parse_if(struct Parser *const parser) {
	size_t line = parser->lex.line;
	YASL_PARSE_DEBUG_LOG("parsing if in line %" PRI_SIZET "\n", line);
	if (matcheattok(parser, T_IF)) ;
	else if (matcheattok(parser, T_ELSEIF)) ;
	else {
		parser_print_err_syntax(parser, "Expected `if` or `elseif`, got `%s` (line %" PRI_SIZET ")\n", YASL_TOKEN_NAMES[curtok(parser)], line);
		return handle_error(parser);
	}
	struct Node *cond = parse_expr(parser);
	struct Node *then_block = parse_body(parser);
	if (curtok(parser) != T_ELSE && curtok(parser) != T_ELSEIF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "no else");
		return new_If(cond, new_Block(then_block, line), NULL, line);
	}
	if (curtok(parser) == T_ELSEIF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "elseif");
		return new_If(cond, new_Block(then_block, line), parse_if(parser), line);
	}
	if (matcheattok(parser, T_ELSE)) {
		size_t else_line = parser->lex.line;
		YASL_PARSE_DEBUG_LOG("%s\n", "else");
		struct Node *else_block = parse_body(parser);
		return new_If(cond, new_Block(then_block, line), new_Block(else_block, else_line), line);
	}
	parser_print_err_syntax(parser, "Expected newline, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
	return handle_error(parser);
}

static struct Node *parse_assert(struct Parser *const parser) {
	size_t line = parser->lex.line;
	eattok(parser, T_ASS);
	return new_Assert(parse_expr(parser), line);
}

static struct Node *parse_expr(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "parsing expression.");
	struct Node *node = parse_ternary(parser);
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
	size_t line = parser->lex.line;\
	YASL_PARSE_DEBUG_LOG("parsing " msg " in line %" PRI_SIZET "\n", parser->lex.line);\
        struct Node *cur_node = parse_##next(parser);\
        if (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
                enum Token op = eattok(parser, curtok(parser));\
                return new_BinOp(op, cur_node, parse_##name(parser), line);\
        }\
        return cur_node;\
}

#define BINOP_L(name, next, msg, ...) \
static struct Node *parse_##name(struct Parser *const parser) {\
	size_t line = parser->lex.line;\
	YASL_PARSE_DEBUG_LOG("parsing " msg " in line %" PRI_SIZET "\n", line);\
        struct Node *cur_node = parse_##next(parser);\
        while (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
                enum Token op = eattok(parser, curtok(parser));\
                cur_node = new_BinOp(op, cur_node, parse_##next(parser), line);\
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
	size_t line = parser->lex.line;
	YASL_PARSE_DEBUG_LOG("parsing !, -, +, ^, len in line %" PRI_SIZET "\n", parser->lex.line);
	if (curtok(parser) == T_PLUS || curtok(parser) == T_MINUS || curtok(parser) == T_BANG ||
	    curtok(parser) == T_CARET || curtok(parser) == T_LEN) {
		enum Token op = eattok(parser, curtok(parser));
		return new_UnOp(op, parse_unary(parser), line);
	} else {
		return parse_power(parser);
	}
}

static struct Node *parse_power(struct Parser *const parser) {
	size_t line = parser->lex.line;
	YASL_PARSE_DEBUG_LOG("parsing ** in line %" PRI_SIZET "\n", parser->lex.line);
	struct Node *cur_node = parse_call(parser);
	if (matcheattok(parser, T_DSTAR)) {
		return new_BinOp(T_DSTAR, cur_node, parse_unary(parser), line);
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
				cur_node = new_Set(cur_node, Call_get_params(right)->children[0],
						   Call_get_params(right)->children[1], parser->lex.line);
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
			if (matcheattok(parser, T_COLON)) {
				struct Node *start = new_Undef(line);
				struct Node *end = TOKEN_MATCHES(parser, T_RSQB) ? new_Undef(line) : parse_expr(parser);
				cur_node = new_Slice(cur_node, start, end, line);
			} else {
				struct Node *expr = parse_expr(parser);
				if (matcheattok(parser, T_COLON)) {
					struct Node *end = TOKEN_MATCHES(parser, T_RSQB) ? new_Undef(line) : parse_expr(parser);
					cur_node = new_Slice(cur_node, expr, end, line);
				} else {
					cur_node = new_Get(cur_node, expr, line);
				}
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
	case T_DOT: {
		eattok(parser, T_DOT);
		struct Node *cur_node = new_String(parser->lex.value, strlen(parser->lex.value), parser->lex.line);
		eattok(parser, T_ID);
		return cur_node;
	}
	case T_ID:
		return parse_id(parser);
	case T_LPAR: {
		eattok(parser, T_LPAR);
		struct Node *expr = parse_expr(parser);
		eattok(parser, T_RPAR);
		return expr;
	}
	case T_LSQB:
		return parse_list(parser);
	case T_LBRC:
		return parse_table(parser);
	case T_STR:
		return parse_string(parser);
	case T_INT:
		return parse_integer(parser);
	case T_FLOAT:
		return parse_float(parser);
	case T_BOOL:
		return parse_boolean(parser);
	case T_UNDEF:
		return parse_undef(parser);
	case T_FN:
		return parse_lambda(parser);
		// handle invalid expressions with sensible error messages.
	case T_ECHO:
	case T_WHILE:
	case T_BREAK:
	case T_RET:
	case T_CONT:
	case T_IF:
	case T_ELSEIF:
	case T_ELSE:
		parser_print_err_syntax(parser, "Expected expression, got `%s` (line %"
			PRI_SIZET
			").\n",
					YASL_TOKEN_NAMES[curtok(parser)], parser->lex.line);
		return handle_error(parser);
	case T_UNKNOWN:
		parser->status = parser->lex.status;
		return NULL;
	default:
		parser_print_err_syntax(parser, "Invalid expression `%s` (line %"
			PRI_SIZET
			").\n", YASL_TOKEN_NAMES[curtok(parser)], parser->lex.line);
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
	size_t line = parser->lex.line;
	eattok(parser, T_LBRC);
	struct Node *keys = new_Body(line);

	// empty table
	if (matcheattok(parser, T_RBRC)) {
		YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table");
		return new_Table(keys, line);
	}

	body_append(&keys, parse_expr(parser));

	// non-empty table
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table");
	eattok(parser, T_COLON);
	body_append(&keys, parse_expr(parser));

	while (parser->lex.c == '\n') eattok(parser, T_SEMI);
	if (matcheattok(parser, T_FOR)) {
		struct Node *iter = parse_iterate(parser);

		struct Node *cond = NULL;
		if (matcheattok(parser, T_IF)) {
			cond = parse_expr(parser);
		}

		while (parser->lex.c == '\n') eattok(parser, T_SEMI);
		eattok(parser, T_RBRC);
		struct Node *table_comp = new_TableComp(keys, iter, cond, line);
		return table_comp;
	}
	while (matcheattok(parser, T_COMMA)) {
		body_append(&keys, parse_expr(parser));
		eattok(parser, T_COLON);
		body_append(&keys, parse_expr(parser));
	}
	while (parser->lex.c == '\n') eattok(parser, T_SEMI);
	eattok(parser, T_RBRC);
	return new_Table(keys, line);
}


// parse list literal
static struct Node *parse_list(struct Parser *const parser) {
	size_t line = parser->lex.line;
	eattok(parser, T_LSQB);
	struct Node *keys = new_Body(line);

	// empty list
	if (matcheattok(parser, T_RSQB)) {
		YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list");
		return new_List(keys, line);
	}

	body_append(&keys, parse_expr(parser));

	while (parser->lex.c == '\n') eattok(parser, T_SEMI);

	// non-empty list
	if (matcheattok(parser, T_FOR)) {
		struct Node *iter = parse_iterate(parser);

		struct Node *cond = NULL;
		if (matcheattok(parser, T_IF)) {
			cond = parse_expr(parser);
		}

		while (parser->lex.c == '\n') eattok(parser, T_SEMI);
		eattok(parser, T_RSQB);
		struct Node *table_comp = new_ListComp(keys->children[0], iter, cond, line);
		free(keys);
		return table_comp;
	} else {
		while (matcheattok(parser, T_COMMA)) {
			YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list");
			body_append(&keys, parse_expr(parser));
		}
		while (parser->lex.c == '\n') eattok(parser, T_SEMI);
		eattok(parser, T_RSQB);
		return new_List(keys, line);
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
