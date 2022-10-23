#include "parser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <opcode.h>

#include "ast.h"
#include "debug.h"
#include "middleend.h"
#include "yasl_conf.h"
#include "yasl_error.h"
#include "yasl_include.h"


static struct Node *parse_program(struct Parser *const parser);
static struct Node *parse_const_fn(struct Parser *const parser);
static struct Node *parse_let(struct Parser *const parser);
static struct Node *parse_decl(struct Parser *const parser);
static struct Node *parse_fn(struct Parser *const parser);
static struct Node *parse_return(struct Parser *const parser);
static struct Node *parse_for(struct Parser *const parser);
static struct Node *parse_while(struct Parser *const parser);
static struct Node *parse_match(struct Parser *const parser);
static struct Node *parse_if(struct Parser *const parser);
static struct Node *parse_ifdef(struct Parser *const parser);
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
static void parse_exprs_or_vargs(struct Parser *const parser, struct Node **body);

YASL_FORMAT_CHECK static void parser_print_err(struct Parser *parser, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	parser->lex.err.print(&parser->lex.err, fmt, args);
	va_end(args);
}

#define parser_print_err_syntax(parser, format, ...) parser_print_err(parser, "SyntaxError: " format, __VA_ARGS__)

void parser_register_node(struct Parser *parser, struct Node *node) {
	if (!parser->head) {
		parser->head = node;
	} else {
		parser->tail->next = node;
	}
	parser->tail = node;
}

void parser_unregister_node(struct Parser *parser, struct Node *node) {
	if (!parser->head)
		return;

	if (parser->head == node) {
		parser->head = parser->head->next;
		node->next = NULL;
		return;
	}

	struct Node *prev = parser->head;
	struct Node *curr = prev->next;
	while (curr) {
		if (curr == node) {
			prev->next = curr->next;
			node->next = NULL;
			if (node == parser->tail)
				parser->tail = prev;
			break;
		}
		prev = curr;
		curr = curr->next;
	}
}

int peof(const struct Parser *const parser) {
	return parser->lex.type == T_EOF;
}

size_t parserline(const struct Parser *const parser) {
	return parser->lex.line;
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
	node_del(parser->head);
	lex_cleanup(&parser->lex);
}

static YASL_NORETURN void handle_error(struct Parser *const parser) {
	parser->status = YASL_SYNTAX_ERROR;
	lex_val_free(&parser->lex);
	lex_val_setnull(&parser->lex);

	while (parser->lex.c != '\n' && !lxeof(parser->lex.file)) {
		lex_getchar(&parser->lex);
	}

	longjmp(parser->env, 1);
}

enum Token eattok(struct Parser *const parser, const enum Token token) {
	if (curtok(parser) != token) {
		if (curtok(parser) == T_UNKNOWN) {
			parser->status = parser->lex.status;
		} else {
			parser_print_err_syntax(parser, "Expected %s, got %s (line %" PRI_SIZET ").\n", YASL_TOKEN_NAMES[token],
						YASL_TOKEN_NAMES[curtok(parser)], parserline(parser));
			parser->status = YASL_SYNTAX_ERROR;
		}
		while (!TOKEN_MATCHES(parser, T_SEMI, T_EOF)) {
			lex_val_free(&parser->lex);
			gettok(&parser->lex);
		}
		return T_UNKNOWN;
	} else {
		gettok(&parser->lex);
	}
	return token;
}

static char *eatname(struct Parser *const parser) {
	char *tmp = lex_val_get(&parser->lex);
	eattok(parser, T_ID);
	if (parser->status)
		handle_error(parser);
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
	if (setjmp(parser->env)) {
		return NULL;
	}
	return parse_program(parser);
}

static struct Node *parse_decl_helper(struct Parser *const parser, struct Node *lvals, size_t i);

struct Node *parse_assign_or_exprstmt(struct Parser *const parser) {
	size_t line = parserline(parser);
	struct Node *expr = parse_expr(parser);

	if (curtok(parser) == T_EQ || tok_isaugmented(curtok(parser))) {
		return parse_assign(parser, expr);
	}

	if (curtok(parser) == T_COMMA && expr->nodetype == N_GET) {
		struct Node *buffer = new_Body(parser, parserline(parser));

		struct Node *set = new_Set(parser, Get_get_collection(expr), Get_get_value(expr), NULL, line);
		body_append(parser, &buffer, set);

		return parse_decl_helper(parser, buffer, 0);

	}

	return new_ExprStmt(parser, expr, line);
}

/*
 * Checks for function statement `fn <id> ...` vs function expr `fn ( ...`.
 */
static bool isfndecl(struct Parser *const parser) {
	long curr = lxtell(parser->lex.file);
	eattok(parser, T_FN);
	lex_val_free(&parser->lex);
	lex_val_setnull(&parser->lex);
	bool result = TOKEN_MATCHES(parser, T_ID);
	lxseek(parser->lex.file, curr, SEEK_SET);
	parser->lex.type = T_FN;
	return result;
}

/*
 * Checks for function statement `fn <id> ...` vs function expr `fn ( ...`.
 */
static bool isemptyvargs(struct Parser *const parser) {
	long curr = lxtell(parser->lex.file);
	eattok(parser, T_LPAR);
	lex_val_free(&parser->lex);
	lex_val_setnull(&parser->lex);
	bool result = TOKEN_MATCHES(parser, T_RPAR);
	lxseek(parser->lex.file, curr, SEEK_SET);
	parser->lex.type = T_LPAR;
	return result;
}

/*
 * Checks for const function statement `const fn ...` vs const decl `const <id> ...`.
 */
static bool isconstfndecl(struct Parser *const parser) {
	long curr = lxtell(parser->lex.file);
	eattok(parser, T_CONST);
	lex_val_free(&parser->lex);
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
	YASL_ByteBuffer buffer;
	lex_val_save(&parser->lex, &buffer);
	lex_val_setnull(&parser->lex);
	eattok(parser, T_ID);
	bool result = TOKEN_MATCHES(parser, T_COMMA);
	lxseek(parser->lex.file, curr, SEEK_SET);
	lex_val_free(&parser->lex);
	parser->lex.type = T_ID;
	lex_val_restore(&parser->lex, &buffer);
	return result;
}

static struct Node *parse_program(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing statement in line %" PRI_SIZET "\n", parserline(parser));
	size_t line = parserline(parser);
	switch (curtok(parser)) {
	case T_ECHO: {
		eattok(parser, T_ECHO);
		struct Node *body = new_Body(parser, parserline(parser));
		parse_exprs_or_vargs(parser, &body);
		return new_Echo(parser, body, line);
	}
	case T_FN:
		if (isfndecl(parser)) return parse_fn(parser);
		else return parse_assign_or_exprstmt(parser);
	case T_RET:
		return parse_return(parser);
	case T_EXPORT:
		eattok(parser, T_EXPORT);
		return new_Export(parser, parse_expr(parser), line);
	case T_CONST:
		if (isconstfndecl(parser)) return parse_const_fn(parser);
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
		return new_Break(parser, line);
	case T_CONT:
		eattok(parser, T_CONT);
		return new_Continue(parser, line);
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
		handle_error(parser);
		break;
	case T_IFDEF:
		return parse_ifdef(parser);
	case T_ELSEIFDEF:
		parser_print_err_syntax(parser,
					"`%s` without previous `ifdef` (line %" PRI_SIZET ").\n",
					YASL_TOKEN_NAMES[curtok(parser)],
					line);
		handle_error(parser);
		break;
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
	size_t line = parserline(parser);
	eattok(parser, T_LBRC);
	struct Node *body = new_Body(parser, line);
	while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
		body_append(parser, &body, parse_program(parser));
		eattok(parser, T_SEMI);
	}
	eattok(parser, T_RBRC);
	return body;
}

static struct Node *parse_expr_or_vargs(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "parsing expression or vargs.");
	size_t line = parserline(parser);
	if (matcheattok(parser, T_TDOT)) {
		return new_Vargs(parser, line);
	} else {
		return parse_expr(parser);
	}
}


static void parse_exprs_or_vargs(struct Parser *const parser, struct Node **body) {
	if (TOKEN_MATCHES(parser, T_LPAR) && isemptyvargs(parser)) {
		eattok(parser, T_LPAR);
		eattok(parser, T_RPAR);
		return;
	}

	struct Node *expr = NULL;
	do {
		expr = parse_expr_or_vargs(parser);
		body_append(parser, body, expr);
	} while (expr->nodetype != N_VARGS && matcheattok(parser, T_COMMA));

	struct Node *last = body_last(*body);
	if (last && will_var_expand(last)) {
		(*body)->children[(*body)->children_len - 1] = new_VariadicContext(last, -1);
	}
}

static struct Node *parse_return_vals(struct Parser *const parser) {
	struct Node *body = new_Body(parser, parserline(parser));
	parse_exprs_or_vargs(parser, &body);
	return body;
}

static struct Node *parse_fn_body(struct Parser *const parser, bool collect_rest_params) {
	struct Node *body = new_Body(parser, parserline(parser));
	if (collect_rest_params)
		body_append(parser, &body, new_CollectRestParams(parser, parserline(parser)));
	if (matcheattok(parser, T_RIGHT_ARR)) {
		// fix the case where we have ... -> fn(...) -> ...;
		// `fn` would otherwise be parsed as an <id> since it follows
		// a -> token.
		if (TOKEN_MATCHES(parser, T_ID)) {
			YASLKeywords(&parser->lex);
		}
		size_t line = parserline(parser);
		if (matcheattok(parser, T_LPAR)) {
			if (matcheattok(parser, T_RPAR)) {
				body_append(parser, &body, new_Return(parser, new_Block(parser, new_Body(parser, line), line), line));
			} else {
				struct Node *block = parse_return_vals(parser);
				eattok(parser, T_RPAR);
				body_append(parser, &body, new_Return(parser, block, line));
			}
		} else {
			struct Node *expr = parse_expr_or_vargs(parser);
			body_append(parser, &body, new_Return(parser, new_VariadicContext(expr, -1), line));
		}
	} else {
		eattok(parser, T_LBRC);
		while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
			body_append(parser, &body, parse_program(parser));
			eattok(parser, T_SEMI);
		}
		eattok(parser, T_RBRC);
	}

	return body;
}

static struct Node *parse_function_params(struct Parser *const parser, bool *collect_rest_params) {
	struct Node *block = new_Body(parser, parserline(parser));
	while (TOKEN_MATCHES(parser, T_ID, T_CONST, T_TDOT)) {
		if (TOKEN_MATCHES(parser, T_ID)) {
			struct Node *cur_node = parse_id(parser);
			cur_node->nodetype = N_LET;
			body_append(parser, &block, cur_node);
		} else if (matcheattok(parser, T_CONST)) {
			struct Node *cur_node = parse_id(parser);
			cur_node->nodetype = N_CONST;
			body_append(parser, &block, cur_node);
		} else {
			eattok(parser, T_TDOT);
			body_append(parser, &block, new_Vargs(parser, parserline(parser)));
			*collect_rest_params = true;
			break;
		}
		if (matcheattok(parser, T_LSQB)) {
			eattok(parser, T_RSQB);
			*collect_rest_params = true;
			break;
		}
		if (!matcheattok(parser, T_COMMA)) break;
	}
	return block;
}

static struct Node *parse_return(struct Parser *const parser) {
	size_t line = parserline(parser);
	eattok(parser, T_RET);
	struct Node *block = parse_return_vals(parser);
	return new_Return(parser, block, line);
}

static struct Node *parse_fn(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing fn in line %" PRI_SIZET "\n", parserline(parser));
	eattok(parser, T_FN);
	size_t line = parserline(parser);
	char *name = eatname(parser);
	if (matcheattok(parser, T_DOT)) {
		struct Node *collection = new_Var(parser, name, line);
		size_t line = parserline(parser);
		char *name = eatname(parser);
		size_t name_len = strlen(name);

		struct Node *index = new_String(parser, name, name_len, line);

		eattok(parser, T_LPAR);
		bool collect_rest_params = false;
		struct Node *block = parse_function_params(parser, &collect_rest_params);
		eattok(parser, T_RPAR);

		struct Node *body = parse_fn_body(parser, collect_rest_params);

		return new_Set(parser, collection, index, new_FnDecl(parser, block, body, NULL, line), line);
	}
	eattok(parser, T_LPAR);
	bool collect_rest_params = false;
	struct Node *block = parse_function_params(parser, &collect_rest_params);
	eattok(parser, T_RPAR);

	struct Node *body = parse_fn_body(parser, collect_rest_params);

	return new_Let(parser, new_FnDecl(parser, block, body, name, line), name, line);
	// TODO Fix this ^
}

static struct Node *parse_const_fn(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing const fn in line %" PRI_SIZET "\n", parserline(parser));
	eattok(parser, T_CONST);
	eattok(parser, T_FN);
	size_t line = parserline(parser);
	char *name = eatname(parser);
	eattok(parser, T_LPAR);
	bool collect_rest_params = false;
	struct Node *block = parse_function_params(parser, &collect_rest_params);
	eattok(parser, T_RPAR);

	struct Node *body = parse_fn_body(parser, collect_rest_params);

	// TODO fix this
	return new_Const(parser, new_FnDecl(parser, block, body, name, line), name, line);
}

static struct Node *parse_let_const_or_var(struct Parser *const parser) {
	size_t line = parserline(parser);
	if (matcheattok(parser, T_LET)) {
		char *name = eatname(parser);
		return new_Let(parser, NULL, name, line);
	} else if (matcheattok(parser, T_CONST)) {
		char *name = eatname(parser);
		return new_Const(parser, NULL, name, line);
	} else {
		struct Node *node = parse_call(parser);
		if (node->nodetype == N_VAR) {
			struct Node *assign = new_Assign(parser, NULL, node->value.sval.str, line);
			return assign;
		} else if (node->nodetype == N_GET) {
			struct Node *set = new_Set(parser, Get_get_collection(node), Get_get_value(node), NULL, line);
			return set;
		} else {
			parser_print_err_syntax(parser, "Expected `let`, `const`, or id, got %s", YASL_TOKEN_NAMES[curtok(parser)]);
			handle_error(parser);
		}
	}
}

static struct Node *parse_var_pack(struct Parser *const parser, int expected) {
	struct Node *rvals = new_Body(parser, parserline(parser));

	int j = 0;
	do {
		body_append(parser, &rvals, parse_expr(parser));
	} while (j++ < expected && matcheattok(parser, T_COMMA));

	struct Node *last = body_last(rvals);
	if (last && !will_var_expand(last)) {
		while (j++ < expected) {
			body_append(parser, &rvals, new_Undef(parser, parserline(parser)));
		}
	} else {
		rvals->children[rvals->children_len - 1] = new_VariadicContext(last, expected - j + 1);
	}

	return rvals;
}

static struct Node *parse_decl_helper(struct Parser *const parser, struct Node *lvals, size_t i) {
	while (matcheattok(parser, T_COMMA)) {
		struct Node *lval = parse_let_const_or_var(parser);
		body_append(parser, &lvals, lval);
		i++;
	}

	eattok(parser, T_EQ);

	struct Node *rvals = parse_var_pack(parser, (int)i);

	return new_Decl(parser, lvals, rvals, lvals->line);
}

static struct Node *parse_decl(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing let in line %" PRI_SIZET "\n", parserline(parser));
	size_t i = 0;

	struct Node *lval = parse_let_const_or_var(parser);
	struct Node *buffer = new_Body(parser, parserline(parser));
	body_append(parser, &buffer, lval);
	i++;

	return parse_decl_helper(parser, buffer, i);
}

static struct Node *parse_let(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing let in line %" PRI_SIZET "\n", parserline(parser));
	eattok(parser, T_LET);
	size_t line = parserline(parser);
	char *name = eatname(parser);
	eattok(parser, T_EQ);
	struct Node *expr = parse_expr(parser);
	return new_Let(parser, expr, name, line);
}

static struct Node *parse_iterate(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing let <- in line %" PRI_SIZET "\n", parserline(parser));
	size_t line = parserline(parser);
	char *name = eatname(parser);
	eattok(parser, T_LEFT_ARR);
	struct Node *collection = parse_expr(parser);
	return new_LetIter(parser, collection, name, line);
}

static struct Node *parse_let_iterate_or_let(struct Parser *const parser) {
	if (curtok(parser) == T_LET) {
		return parse_let(parser);
	} else {
		return parse_iterate(parser);
	}
}

static struct Node *parse_for(struct Parser *const parser) {
	size_t line = parserline(parser);
	eattok(parser, T_FOR);

	struct Node *iter = parse_let_iterate_or_let(parser);

	if (iter->nodetype == N_LETITER) {
		struct Node *body = parse_body(parser);
		return new_ForIter(parser, iter, new_Block(parser, body, line), line);
	} else {
		eattok(parser, T_SEMI);
		struct Node *cond = parse_expr(parser);
		eattok(parser, T_SEMI);
		struct Node *post = parse_assign_or_exprstmt(parser);
		struct Node *body = parse_body(parser);
		struct Node *outer_body = new_Body(parser, line);
		body_append(parser, &outer_body, iter);
		body_append(parser, &outer_body, new_While(parser, cond, new_Block(parser, body, line), new_ExprStmt(parser, post, line), line));
		struct Node *block = new_Block(parser, outer_body, line);
		return block;
	}
}

static struct Node *parse_while(struct Parser *const parser) {
	size_t line = parserline(parser);
	YASL_PARSE_DEBUG_LOG("parsing while in line %" PRI_SIZET "\n", line);
	eattok(parser, T_WHILE);
	struct Node *cond = parse_expr(parser);
	struct Node *body = parse_body(parser);
	return new_While(parser, cond, new_Block(parser, body, line), NULL, line);
}

static struct Node *parse_pattern(struct Parser *const parser);

static struct Node *parse_primitivepattern(struct Parser *const parser) {
	size_t line = parserline(parser);
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
			handle_error(parser);
		}
	case T_PLUS:
		eattok(parser, T_PLUS);
		if (curtok(parser) == T_INT) {
			n = parse_integer(parser);
			n->nodetype = N_PATINT;
			return n;
		} else if (curtok(parser) == T_FLOAT) {
			n = parse_float(parser);
			n->nodetype = N_PATFL;
			return n;
		} else {
			parser_print_err_syntax(parser, "Expected numeric pattern, got pattern starting in %s (line %" PRI_SIZET ").\n", YASL_TOKEN_NAMES[curtok(parser)], line);
			handle_error(parser);
		}
	case T_STR:
		if (parser->lex.mode == L_INTERP) {
			while (parser->lex.c != '"') {
				lex_getchar(&parser->lex);
			}
			eattok(parser, T_STR);
			parser_print_err_syntax(parser, "Interpolated strings cannot be used in patterns (line %" PRI_SIZET ").\n", line);
			handle_error(parser);
		}
		n = parse_string(parser);
		n->nodetype = N_PATSTR;
		return n;
	case T_DOT:
		eattok(parser, T_DOT);
		n = parse_id(parser);
		n->nodetype = N_PATSTR;
		return n;
	case T_ID: {
		char *name = eatname(parser);
		n = new_Undef(parser, line);
		if (!strcmp(name, "bool")) {
			n->nodetype = N_PATBOOLTYPE;
		} else if (!strcmp(name, "int")) {
			n->nodetype = N_PATINTTYPE;
		} else if (!strcmp(name, "float")) {
			n->nodetype = N_PATFLOATTYPE;
		} else if (!strcmp(name, "str")) {
			n->nodetype = N_PATSTRTYPE;
		} else if (!strcmp(name, "list")) {
			n->nodetype = N_PATLSTYPE;
		} else if (!strcmp(name, "table")) {
			n->nodetype = N_PATTABLETYPE;
		} else {
			parser_print_err_syntax(parser, "Invalid pattern: %s (line %" PRI_SIZET ").\n", name, line);
			handle_error(parser);
		}
		free(name);
		return n;
	}
	default:
		parser_print_err_syntax(parser, "Invalid pattern starting in %s (line %" PRI_SIZET ").\n", YASL_TOKEN_NAMES[curtok(parser)], line);
		handle_error(parser);
	}
}

static struct Node *parse_patternsingle(struct Parser *const parser) {
	size_t line = parserline(parser);
	struct Node *n;

	switch (curtok(parser)) {
	case T_STAR:
		eattok(parser, T_STAR);
		n = new_Undef(parser, line);
		n->nodetype = N_PATANY;
		return n;
	case T_UNDEF:
		eattok(parser, T_UNDEF);
		n = new_Undef(parser, line);
		n->nodetype = N_PATUNDEF;
		return n;
	case T_LET: {
		eattok(parser, T_LET);
		char *name = eatname(parser);
		n = new_Let(parser, NULL, name, line);
		n->nodetype = N_PATLET;
		return n;
	}
	case T_CONST: {
		eattok(parser, T_CONST);
		char *name = eatname(parser);
		n = new_Const(parser, NULL, name, line);
		n->nodetype = N_PATCONST;
		return n;
	}
	case T_LBRC: {
		eattok(parser, T_LBRC);
		n = new_Body(parser, line);
		n->nodetype = N_PATTABLE;
		if (matcheattok(parser, T_TDOT)) {
			n->nodetype = N_PATVTABLE;
		} else if (curtok(parser) != T_RBRC) {
			body_append(parser, &n, parse_primitivepattern(parser));
			eattok(parser, T_COLON);
			body_append(parser, &n, parse_pattern(parser));
			while (matcheattok(parser, T_SEMI)) ;
			while (matcheattok(parser, T_COMMA)) {
				YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table pattern");
				if (matcheattok(parser, T_TDOT)) {
					n->nodetype = N_PATVTABLE;
					break;
				}
				body_append(parser, &n, parse_primitivepattern(parser));
				eattok(parser, T_COLON);
				body_append(parser, &n, parse_pattern(parser));
			}
			while (matcheattok(parser, T_SEMI)) ;
		}
		eattok(parser, T_RBRC);
		return n;
	}
	case T_LSQB:
		eattok(parser, T_LSQB);
		n = new_Body(parser, line);
		n->nodetype = N_PATLS;
		if (matcheattok(parser, T_TDOT)) {
			n->nodetype = N_PATVLS;
		} else if (curtok(parser) != T_RSQB) {
			body_append(parser, &n, parse_pattern(parser));
			while (matcheattok(parser, T_SEMI)) ;
			while (matcheattok(parser, T_COMMA)) {
				YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list pattern");
				if (matcheattok(parser, T_TDOT)) {
					n->nodetype = N_PATVLS;
					break;
				}
				body_append(parser, &n, parse_pattern(parser));
			}
			while (matcheattok(parser, T_SEMI)) ;
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
	size_t line = parserline(parser);
        struct Node *cur_node = parse_patternsingle(parser);
        if (matcheattok(parser, T_BAR)) {
                struct Node *tmp = new_BinOp(parser, T_BAR, cur_node, parse_alt(parser), line);
                tmp->nodetype = N_PATALT;
                return tmp;
        }
        return cur_node;
}

static struct Node *parse_pattern(struct Parser *const parser) {
	return parse_alt(parser);
}

static struct Node *parse_match(struct Parser *const parser) {
	size_t line = parserline(parser);
	YASL_PARSE_DEBUG_LOG("parsing match in line %" PRI_SIZET "\n", line);
	eattok(parser, T_MATCH);
	struct Node *expr = parse_expr(parser);
	(void)expr;
	eattok(parser, T_LBRC);
	struct Node *pats = new_Body(parser, line);
	struct Node *guards = new_Body(parser, line);
	struct Node *bodies = new_Body(parser, line);
	while (curtok(parser) != T_RBRC && curtok(parser) != T_EOF) {
		body_append(parser, &pats, parse_pattern(parser));
		if (matcheattok(parser, T_IF)) {
			body_append(parser, &guards, parse_expr(parser));
		} else {
			body_append(parser, &guards, NULL);
		}
		body_append(parser, &bodies, parse_body(parser));
		eattok(parser, T_SEMI);
	}
	eattok(parser, T_RBRC);
	return new_Match(parser, expr, pats, guards, bodies, line);
}

static struct Node *parse_if(struct Parser *const parser) {
	size_t line = parserline(parser);
	YASL_PARSE_DEBUG_LOG("parsing if in line %" PRI_SIZET "\n", line);
	if (matcheattok(parser, T_IF)) ;
	else if (matcheattok(parser, T_ELSEIF)) ;
	else {
		parser_print_err_syntax(parser, "Expected `if` or `elseif`, got `%s` (line %" PRI_SIZET ")\n", YASL_TOKEN_NAMES[curtok(parser)], line);
		handle_error(parser);
	}
	struct Node *cond = parse_expr(parser);
	struct Node *then_block = parse_body(parser);
	if (curtok(parser) != T_ELSE && curtok(parser) != T_ELSEIF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "no else");
		return new_If(parser, cond, new_Block(parser, then_block, line), NULL, line);
	}
	if (curtok(parser) == T_ELSEIF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "elseif");
		return new_If(parser, cond, new_Block(parser, then_block, line), parse_if(parser), line);
	}
	if (matcheattok(parser, T_ELSE)) {
		size_t else_line = parserline(parser);
		YASL_PARSE_DEBUG_LOG("%s\n", "else");
		struct Node *else_block = parse_body(parser);
		return new_If(parser, cond, new_Block(parser, then_block, line), new_Block(parser, else_block, else_line), line);
	}
	parser_print_err_syntax(parser, "Expected newline, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
	handle_error(parser);
}

static struct Node *parse_ifdef(struct Parser *const parser) {
	size_t line = parserline(parser);
	YASL_PARSE_DEBUG_LOG("parsing `ifdef` in line %" PRI_SIZET "\n", line);
	if (matcheattok(parser, T_IFDEF)) ;
	else if (matcheattok(parser, T_ELSEIFDEF));
	else {
		parser_print_err_syntax(parser, "Expected `ifdef` or `elseifdef`, got `%s` (line %" PRI_SIZET ")\n", YASL_TOKEN_NAMES[curtok(parser)], line);
		handle_error(parser);
	}
	struct Node *cond = parse_id(parser);
	struct Node *then_block = parse_body(parser);
	if (curtok(parser) != T_ELSE && curtok(parser) != T_ELSEIFDEF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "no else");
		return new_IfDef(parser, cond, new_Block(parser, then_block, line), NULL, line);
	}
	if (curtok(parser) == T_ELSEIFDEF) {
		YASL_PARSE_DEBUG_LOG("%s\n", "elseifdef");
		return new_IfDef(parser, cond, new_Block(parser, then_block, line), parse_ifdef(parser), line);
	}
	if (matcheattok(parser, T_ELSE)) {
		size_t else_line = parserline(parser);
		YASL_PARSE_DEBUG_LOG("%s\n", "else");
		struct Node *else_block = parse_body(parser);
		return new_IfDef(parser, cond, new_Block(parser, then_block, line), new_Block(parser, else_block, else_line), line);
	}
	parser_print_err_syntax(parser, "Expected newline, got `%s`.\n", YASL_TOKEN_NAMES[curtok(parser)]);
	handle_error(parser);
}

static struct Node *parse_assert(struct Parser *const parser) {
	size_t line = parserline(parser);
	eattok(parser, T_ASS);
	return new_Assert(parser, parse_expr(parser), line);
}

static struct Node *parse_expr(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "parsing expression.");
	struct Node *node = parse_ternary(parser);
	fold(node);
	return node;
}

static struct Node *parse_assign(struct Parser *const parser, struct Node *cur_node) {
	YASL_PARSE_DEBUG_LOG("parsing = in line %" PRI_SIZET "\n", parserline(parser));
	size_t line = parserline(parser);
	if (matcheattok(parser, T_EQ)) {
		switch (cur_node->nodetype) {
		case N_VAR: {
			struct Node *assign_node = new_Assign(parser, parse_expr(parser), cur_node->value.sval.str, line);
			return assign_node;
		}
		case N_GET: {
			struct Node *left = cur_node->children[0];
			struct Node *key = cur_node->children[1];
			struct Node *val = parse_expr(parser);
			return new_Set(parser, left, key, val, line);
		}
		default:
			parser_print_err_syntax(parser, "Invalid l-value (line %" PRI_SIZET ").\n", line);
			handle_error(parser);
		}
	} else if (tok_isaugmented(curtok(parser))) {
	  enum Token op = (enum Token)(eattok(parser, curtok(parser)) - 1);  // relies on enum in lexer.h
		switch (cur_node->nodetype) {
		case N_VAR: {
			struct Node *tmp = new_BinOp(parser, op, cur_node, parse_expr(parser), line);
			return new_Assign(parser, tmp, cur_node->value.sval.str, line);
		}
		case N_GET: {
			struct Node *collection = cur_node->children[0];
			struct Node *key = cur_node->children[1];
			struct Node *value = new_BinOp(parser, op, cur_node, parse_expr(parser), line);
			return new_Set(parser, collection, key, value, line);
		}
		default:
			parser_print_err_syntax(parser, "Invalid l-value (line %" PRI_SIZET ").\n", line);
			handle_error(parser);
		}
	}
	return cur_node;
}

static struct Node *parse_ternary(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("parsing ?: in line %" PRI_SIZET "\n", parserline(parser));
	struct Node *cur_node = parse_undef_or(parser);
	if (matcheattok(parser, T_QMARK)) {
		struct Node *left = parse_ternary(parser);
		eattok(parser, T_COLON);
		struct Node *right = parse_ternary(parser);
		return new_TriOp(parser, T_QMARK, cur_node, left, right, parserline(parser));
	}
	return cur_node;
}


#define BINOP_R(name, next, msg, ...)\
static struct Node *parse_##name(struct Parser *const parser) {\
	size_t line = parserline(parser);\
	YASL_PARSE_DEBUG_LOG("parsing " msg " in line %" PRI_SIZET "\n", parserline(parser));\
        struct Node *cur_node = parse_##next(parser);\
        if (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
                enum Token op = eattok(parser, curtok(parser));\
                return new_BinOp(parser, op, cur_node, parse_##name(parser), line);\
        }\
        return cur_node;\
}

#define BINOP_L(name, next, msg, ...) \
static struct Node *parse_##name(struct Parser *const parser) {\
	size_t line = parserline(parser);\
	YASL_PARSE_DEBUG_LOG("parsing " msg " in line %" PRI_SIZET "\n", line);\
        struct Node *cur_node = parse_##next(parser);\
        while (TOKEN_MATCHES(parser, __VA_ARGS__)) {\
                enum Token op = eattok(parser, curtok(parser));\
                cur_node = new_BinOp(parser, op, cur_node, parse_##next(parser), line);\
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
	size_t line = parserline(parser);
	YASL_PARSE_DEBUG_LOG("parsing !, -, +, ^, len in line %" PRI_SIZET "\n", parserline(parser));
	if (curtok(parser) == T_PLUS || curtok(parser) == T_MINUS || curtok(parser) == T_BANG ||
	    curtok(parser) == T_CARET || curtok(parser) == T_LEN) {
		enum Token op = eattok(parser, curtok(parser));
		return new_UnOp(parser, op, parse_unary(parser), line);
	} else {
		return parse_power(parser);
	}
}

static struct Node *parse_power(struct Parser *const parser) {
	size_t line = parserline(parser);
	YASL_PARSE_DEBUG_LOG("parsing ** in line %" PRI_SIZET "\n", parserline(parser));
	struct Node *cur_node = parse_call(parser);
	if (matcheattok(parser, T_DSTAR)) {
		return new_BinOp(parser, T_DSTAR, cur_node, parse_unary(parser), line);
	}
	return cur_node;
}

static struct Node *parse_call(struct Parser *const parser) {
	struct Node *cur_node = parse_constant(parser);
	while (TOKEN_MATCHES(parser, T_LSQB, T_DOT, T_LPAR, T_RIGHT_ARR)) {
		if (matcheattok(parser, T_RIGHT_ARR)) {
			struct Node *right = parse_constant(parser);
			if (right->nodetype != N_VAR) {
				parser_print_err_syntax(parser, "Invalid method call (line %" PRI_SIZET ").\n", parserline(parser));
				handle_error(parser);
			}

			struct Node *block = new_Body(parser, parserline(parser));

			cur_node = new_MethodCall(parser, block, cur_node, right->value.sval.str, 1,
						  parserline(parser));
			eattok(parser, T_LPAR);
			if (!TOKEN_MATCHES(parser, T_RPAR, T_EOF)) {
				parse_exprs_or_vargs(parser, &cur_node->children[0]);
			}
			eattok(parser, T_RPAR);

			struct Node *body = cur_node->children[0];
			struct Node *last = body_last(body);
			if (last && will_var_expand(last)) {
				body->children[body->children_len - 1] = new_VariadicContext(last, -1);
			}
		} else if (matcheattok(parser, T_DOT)) {
			struct Node *right = parse_constant(parser);
			if (right->nodetype == N_CALL) {
				cur_node = new_Set(parser, cur_node, Call_get_params(right)->children[0],
						   Call_get_params(right)->children[1], parserline(parser));
			} else if (right->nodetype == N_VAR) {
				right->nodetype = N_STR;
				cur_node = new_Get(parser, cur_node, right, parserline(parser));
			} else {
				parser_print_err_syntax(parser, "Invalid member access (line %" PRI_SIZET ").\n", parserline(parser));
				handle_error(parser);
			}
		} else if (matcheattok(parser, T_LSQB)) {
			size_t line = parserline(parser);
			if (matcheattok(parser, T_COLON)) {
				struct Node *start = new_Undef(parser, line);
				struct Node *end = TOKEN_MATCHES(parser, T_RSQB) ? new_Undef(parser, line) : parse_expr(parser);
				cur_node = new_Slice(parser, cur_node, start, end, line);
			} else {
				struct Node *expr = parse_expr(parser);
				if (matcheattok(parser, T_COLON)) {
					struct Node *end = TOKEN_MATCHES(parser, T_RSQB) ? new_Undef(parser, line) : parse_expr(parser);
					cur_node = new_Slice(parser, cur_node, expr, end, line);
				} else {
					cur_node = new_Get(parser, cur_node, expr, line);
				}
			}
			eattok(parser, T_RSQB);
		} else if (matcheattok(parser, T_LPAR)) {
			YASL_PARSE_DEBUG_LOG("%s\n", "Parsing function call");
			cur_node = new_Call(parser, new_Body(parser, parserline(parser)), cur_node, 1, parserline(parser));
			if (!TOKEN_MATCHES(parser, T_RPAR, T_EOF)) {
				parse_exprs_or_vargs(parser, &cur_node->children[0]);
			}
			eattok(parser, T_RPAR);

			struct Node *body = cur_node->children[0];
			struct Node *last = body_last(body);
			if (last && will_var_expand(last)) {
				body->children[body->children_len - 1] = new_VariadicContext(last, -1);
			}
		}
	}
	return cur_node;
}

static struct Node *parse_constant(struct Parser *const parser) {
	switch (curtok(parser)) {
	case T_DOT: {
		eattok(parser, T_DOT);
		size_t line = parserline(parser);
		const char *name = eatname(parser);
		struct Node *cur_node = new_String(parser, (char *)name, strlen(name), line);
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
	case T_FOR:
	case T_LET:
	case T_CONST:
	case T_BREAK:
	case T_RET:
	case T_CONT:
	case T_IF:
	case T_ELSEIF:
	case T_ELSE:
		parser_print_err_syntax(parser, "Expected expression, got `%s` (line %"
			PRI_SIZET
			").\n",
					YASL_TOKEN_NAMES[curtok(parser)], parserline(parser));
		handle_error(parser);
	case T_UNKNOWN:
		handle_error(parser);
	default:
		parser_print_err_syntax(parser, "Invalid expression `%s` (line %"
			PRI_SIZET
			").\n", YASL_TOKEN_NAMES[curtok(parser)], parserline(parser));
		handle_error(parser);
	}
}

static struct Node *parse_id(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing variable");
	size_t line = parserline(parser);
	char *name = eatname(parser);
	struct Node *cur_node = new_Var(parser, name, line);
	return cur_node;
}

static struct Node *parse_undef(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing undef");
	struct Node *cur_node = new_Undef(parser, parserline(parser));
	eattok(parser, T_UNDEF);
	return cur_node;
}

static struct Node *parse_lambda(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing lambda");
	size_t line = parserline(parser);

	eattok(parser, T_FN);
	eattok(parser, T_LPAR);
	bool collect_rest_params = false;
	struct Node *block = parse_function_params(parser, &collect_rest_params);
	eattok(parser, T_RPAR);
	struct Node *body = parse_fn_body(parser, collect_rest_params);

	return new_FnDecl(parser, block, body, NULL, line);
}

static yasl_float get_float(char *buffer) {
	return strtod(buffer, (char **) NULL);
}

static struct Node *parse_float(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing float");
	struct Node *cur_node = new_Float(parser, get_float(lex_val_get(&parser->lex)), parserline(parser));
	lex_val_free(&parser->lex);
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
	struct Node *cur_node = new_Integer(parser, get_int(lex_val_get(&parser->lex)), parserline(parser));
	lex_val_free(&parser->lex);
	eattok(parser, T_INT);
	return cur_node;
}

static struct Node *parse_boolean(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing bool");
	struct Node *cur_node = new_Boolean(parser, !strcmp(lex_val_get(&parser->lex), "true"), parserline(parser));
	lex_val_free(&parser->lex);
	eattok(parser, T_BOOL);
	return cur_node;
}

static struct Node *parse_string(struct Parser *const parser) {
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing str");
	struct Node *cur_node = new_String(parser, lex_val_get(&parser->lex), parser->lex.buffer.count, parserline(parser));

	// interpolated strings
	while (parser->lex.mode == L_INTERP) {
		eattok(parser, T_STR);
		eattok(parser, T_LBRC);
		parser->lex.mode = L_NORMAL;
		struct Node *expr = parse_expr(parser);
		parser->lex.mode = L_INTERP;
		cur_node = new_BinOp(parser, T_TILDE, cur_node, expr, parserline(parser));
		if (parser->lex.c == '}') {
			parser->lex.c = lxgetc(parser->lex.file);
		} else {
			parser_print_err_syntax(parser, "Expected } in line %" PRI_SIZET ".\n", parserline(parser));
			handle_error(parser);
		}
		lex_eatinterpstringbody(&parser->lex);
		if (parser->lex.status) {
			handle_error(parser);
		};
		struct Node *str = new_String(parser, lex_val_get(&parser->lex), parser->lex.buffer.count, parserline(parser));
		cur_node = new_BinOp(parser, T_TILDE, cur_node, str, parserline(parser));
	}

	eattok(parser, T_STR);

	return cur_node;
}


static struct Node *parse_table(struct Parser *const parser) {
	size_t line = parserline(parser);
	eattok(parser, T_LBRC);
	struct Node *keys = new_Body(parser, line);

	// empty table
	if (matcheattok(parser, T_RBRC)) {
		YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table");
		return new_Table(parser, keys, line);
	}

	body_append(parser, &keys, parse_expr(parser));

	// non-empty table
	YASL_PARSE_DEBUG_LOG("%s\n", "Parsing table");
	eattok(parser, T_COLON);
	body_append(parser, &keys, parse_expr(parser));

	while (matcheattok(parser, T_SEMI)) ;
	if (matcheattok(parser, T_FOR)) {
		struct Node *iter = parse_iterate(parser);

		struct Node *cond = NULL;
		if (matcheattok(parser, T_IF)) {
			cond = parse_expr(parser);
		}

		while (matcheattok(parser, T_SEMI)) ;
		eattok(parser, T_RBRC);
		struct Node *table_comp = new_TableComp(parser, keys, iter, cond, line);
		return table_comp;
	}
	while (matcheattok(parser, T_COMMA)) {
		body_append(parser, &keys, parse_expr(parser));
		eattok(parser, T_COLON);
		body_append(parser, &keys, parse_expr(parser));
	}
	while (matcheattok(parser, T_SEMI)) ;
	eattok(parser, T_RBRC);
	return new_Table(parser, keys, line);
}


// parse list literal
static struct Node *parse_list(struct Parser *const parser) {
	size_t line = parserline(parser);
	eattok(parser, T_LSQB);
	struct Node *keys = new_Body(parser, line);

	// empty list
	if (matcheattok(parser, T_RSQB)) {
		YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list");
		return new_List(parser, keys, line);
	}

	body_append(parser, &keys, parse_expr(parser));

	while (matcheattok(parser, T_SEMI)) ;

	// non-empty list
	if (matcheattok(parser, T_FOR)) {
		struct Node *iter = parse_iterate(parser);

		struct Node *cond = NULL;
		if (matcheattok(parser, T_IF)) {
			cond = parse_expr(parser);
		}

		while (matcheattok(parser, T_SEMI)) ;
		eattok(parser, T_RSQB);
		struct Node *table_comp = new_ListComp(parser, keys->children[0], iter, cond, line);
		return table_comp;
	} else {
		while (matcheattok(parser, T_COMMA)) {
			YASL_PARSE_DEBUG_LOG("%s\n", "Parsing list");
			body_append(parser, &keys, parse_expr(parser));
		}
		while (matcheattok(parser, T_SEMI)) ;
		eattok(parser, T_RSQB);
		return new_List(parser, keys, line);
	}
}
