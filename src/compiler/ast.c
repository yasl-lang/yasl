#include "ast.h"

#include <stdarg.h>

#include "common/debug.h"
#include "yasl_conf.h"

void parser_register_node(struct Parser *parser, struct Node *node);
void parser_unregister_node(struct Parser *parser, struct Node *node);

struct Node *node_clone(const struct Node *const node) {
	if (node == NULL) return NULL;
	struct Node *clone = (struct Node *)malloc(sizeof(struct Node) + node->children_len * sizeof(struct Node *));
	clone->nodetype = node->nodetype;
	clone->children_len = node->children_len;
	for (size_t i = 0; i < clone->children_len; i++) {
		clone->children[i] = node_clone(node->children[i]);
	}

	switch (node->nodetype) {
	case N_BINOP:
		clone->value.binop = node->value.binop;
		clone->value.binop.left = node_clone(node->value.binop.left);
		clone->value.binop.right = node_clone(node->value.binop.right);
		break;
	case N_UNOP:
		clone->value.unop = node->value.unop;
		clone->value.unop.expr = node_clone(node->value.unop.expr);
		break;
	case N_TRIOP:
		clone->value.type = node->value.type;
		break;
	case N_INT:
	case N_BOOL:
	case N_FLOAT:
	case N_CALL:
		clone->value.ival = node->value.ival;
		break;
	default:
		clone->value.sval.len = node->value.sval.len;
		clone->value.sval.str = (char *) malloc(node->value.sval.len + 1);
		clone->value.sval.str[clone->value.sval.len] = '\0';
		memcpy(clone->value.sval.str, node->value.sval.str, clone->value.sval.len);
	}

	clone->line = node->line;
	return clone;
}

static struct Node *new_Node(struct Parser *parser, const enum NodeType nodetype, const size_t line, const size_t name_len,
		char *const name /* OWN */, const size_t n, ... /* OWN */) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * n);
	node->next = NULL;
	node->nodetype = nodetype;
	node->children_len = n;

	node->value.sval.len = name_len;
	node->value.sval.str = name;

	node->line = line;
	va_list children;
	va_start(children, n);
	for (size_t i = 0; i < n; i++) {
		node->children[i] = va_arg(children, struct Node*);
	}
	va_end(children);

	parser_register_node(parser, node);
	return node;
}

#define new_Node_0(parser, nodetype, name, name_len, line) new_Node(parser, nodetype, line, name_len, name, 0)
#define new_Node_1(parser, nodetype, child, name, name_len, line) new_Node(parser, nodetype, line, name_len, name, 1, child)
#define new_Node_2(parser, nodetype, child1, child2, name, name_len, line) new_Node(parser, nodetype, line, name_len, name, 2, child1, child2)
#define new_Node_3(parser, nodetype, child1, child2, child3, name, name_len, line) new_Node(parser, nodetype, line, name_len, name, 3, child1, child2, child3)
#define new_Node_4(parser, nodetype, child1, child2, child3, child4, name, name_len, line) new_Node(parser, nodetype, line, name_len, name, 4, child1, child2, child3, child4)

#define DEF_NODE(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DEF_NODE4, DEF_NODE3, DEF_NODE2, DEF_NODE1, DEF_NODE0)(__VA_ARGS__))
#define DEF_NODE_STR(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DEF_NODE_STR4, DEF_NODE_STR3, DEF_NODE_STR2, DEF_NODE_STR1, DEF_NODE_STR0)(__VA_ARGS__))
#define DEF_NODE_INT(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DEF_NODE_INT4, DEF_NODE_INT3, DEF_NODE_INT2, DEF_NODE_INT1, DEF_NODE_INT0)(__VA_ARGS__))
#define DEF_NODE_ZSTR(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DEF_NODE_ZSTR4, DEF_NODE_ZSTR3, DEF_NODE_ZSTR2, DEF_NODE_ZSTR1, DEF_NODE_ZSTR0)(__VA_ARGS__))
#define DEF_GETTER(name, E, a, n) \
struct Node *name##_get_##a(const struct Node *const node) {\
	YASL_ASSERT(node->nodetype == E, "Expected " #name);\
	return node->children[n];\
}
#define DEF_GETNAME(name, E) \
char *name##_get_name(const struct Node *const node) {\
	YASL_ASSERT(node->nodetype == E, "Expected " #name);\
	return node->value.sval.str;\
}

#define DEF_NODE0(name, E) \
struct Node *new_##name(struct Parser *parser, size_t line) {\
	return new_Node_0(parser, E, NULL, 0, line);\
}

#define DEF_NODE1(name, E, a) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const size_t line) {\
	return new_Node_1(parser, E, a, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)

#define DEF_NODE2(name, E, a, b) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const size_t line) {\
	return new_Node_2(parser, E, a, b, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)

#define DEF_NODE3(name, E, a, b, c) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const struct Node *const c, const size_t line) {\
	return new_Node_3(parser, E, a, b, c, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)\
DEF_GETTER(name, E, c, 2)

#define DEF_NODE4(name, E, a, b, c, d) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const struct Node *const c, const struct Node *const d, const size_t line) {\
	return new_Node_4(parser, E, a, b, c, d, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)\
DEF_GETTER(name, E, c, 2)\
DEF_GETTER(name, E, d, 3)

#define DEF_NODE_STR2(name, E, a, b) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, char *name, size_t name_len, const size_t line) {\
	return new_Node_2(parser, E, a, b, name, name_len, line);\
}\
DEF_GETNAME(name, E)\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)

#define DEF_NODE_INT2(name, E, a, b) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const size_t n, const size_t line) {\
	struct Node *node = new_Node_2(parser, E, a, b, (char *)NULL, 0, line);\
	node->value.ival = n;\
	return node;\
}\
DEF_GETNAME(name, E)\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)

#define DEF_NODE_ZSTR0(name, E) \
struct Node *new_##name(struct Parser *parser, char *name, const size_t line) {\
	return new_Node_0(parser, E, name, name ? strlen(name) : 0, line);\
}\
DEF_GETNAME(name, E)

#define DEF_NODE_ZSTR1(name, E, a) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, char *name, const size_t line) {\
	return new_Node_1(parser, E, a, name, name ? strlen(name) : 0, line);\
}\
DEF_GETNAME(name, E)\
DEF_GETTER(name, E, a, 0)

#define DEF_NODE_ZSTR2(name, E, a, b) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, char *name, const size_t line) {\
	return new_Node_2(parser, E, a, b, name, name ? strlen(name) : 0, line);\
}\
DEF_GETNAME(name, E)\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)


void body_append(struct Parser *parser, struct Node **node, struct Node *const child) {
	YASL_COMPILE_DEBUG_LOG("%s\n", "appending to block");
	parser_unregister_node(parser, *node);
	*node = (struct Node*)realloc(*node, sizeof(struct Node) + (++(*node)->children_len) * sizeof(struct Node *));
	parser_register_node(parser, *node);
	(*node)->children[(*node)->children_len - 1] = child;
}

struct Node *body_last(struct Node *body) {
	return body->children[body->children_len - 1];
}

bool will_var_expand(struct Node *node) {
	return node->nodetype == N_CALL || node->nodetype == N_MCALL || node->nodetype == N_STRINGIFY;
}

DEF_NODE(ExprStmt, N_EXPRSTMT, expr)

struct Node *new_Block(struct Parser *parser, const struct Node *const body, const size_t line) {
	return new_Node_1(parser, N_BLOCK, body, NULL, 0, line);
}

DEF_NODE(CollectRestParams, N_COLLECTRESTPARAMS)
DEF_NODE(Vargs, N_VARGS)
DEF_NODE(Body, N_BODY)
DEF_NODE(Exprs, N_EXPRS)
DEF_NODE(Parens, N_PARENS, expr)
DEF_NODE(Decl, N_DECL, lvals, rvals)
DEF_NODE_ZSTR(FnDecl, N_FNDECL, params, body)
DEF_NODE(Return, N_RET, exprs)
DEF_NODE(Export, N_EXPORT, expr)
DEF_NODE_INT(Call, N_CALL, params, object)
DEF_NODE_STR(MethodCall, N_MCALL, params, object)
DEF_NODE(Set, N_SET, collection, key, value)
DEF_NODE(Get, N_GET, collection, value)
DEF_NODE(Slice, N_SLICE, collection, start, end)
DEF_NODE(ListComp, N_LISTCOMP, expr, iter, cond)
DEF_NODE(TableComp, N_TABLECOMP, expr, iter, cond)
DEF_NODE_ZSTR(LetIter, N_LETITER, collection)
DEF_NODE(ForIter, N_FORITER, iter, body)
DEF_NODE(While, N_WHILE, cond, body, post)
DEF_NODE(Break, N_BREAK)
DEF_NODE(Continue, N_CONT)
DEF_NODE(Match, N_MATCH, cond, patterns, guards, bodies)
DEF_NODE(If, N_IF, cond, then, el)
DEF_NODE(IfDef, N_IFDEF, cond, then, el)
DEF_NODE_ZSTR(Stringify, N_STRINGIFY, expr)
DEF_NODE(Echo, N_ECHO, exprs)
DEF_NODE(Assert, N_ASS, expr)
DEF_NODE_ZSTR(Let, N_LET, expr)
DEF_NODE_ZSTR(Const, N_CONST, expr)

struct Node *new_TriOp(struct Parser *parser, enum Token op, struct Node *left, struct Node *middle, struct Node *right, const size_t line) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * 3);
	node->next = NULL;
	node->nodetype = N_TRIOP;
	node->children_len = 3;
	node->value.type = op;
	node->line = line;
	node->children[0] = left;
	node->children[1] = middle;
	node->children[2] = right;

	parser_register_node(parser, node);
	return node;
}

struct Node *new_BinOp(struct Parser *parser, enum Token op, struct Node *left, struct Node *right, const size_t line) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * 2);
	node->next = NULL;
	node->nodetype = N_BINOP;
	node->children_len = 0;
	node->value.binop = ((struct BinOpNode) { op, left, right });
	node->line = line;

	parser_register_node(parser, node);
	return node;
}

struct Node *new_UnOp(struct Parser *parser, enum Token op, struct Node *child, const size_t line) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * 1);
	node->next = NULL;
	node->nodetype = N_UNOP;
	node->children_len = 0;
	node->value.unop = ((struct UnOpNode) { op, child });
	node->line = line;

	parser_register_node(parser, node);
	return node;
}

DEF_NODE_ZSTR(Assign, N_ASSIGN, expr)
DEF_NODE_ZSTR(Var, N_VAR)
DEF_NODE(Undef, N_UNDEF)

struct Node *new_Float(struct Parser *parser, yasl_float val, const size_t line) {
	struct Node *node = new_Node_0(parser, N_FLOAT, NULL, 0, line);
	node->value.dval = val;
	return node;
}

struct Node *new_Integer(struct Parser *parser, yasl_int val, const size_t line) {
	struct Node *node = new_Node_0(parser, N_INT, NULL, 0, line);
	node->value.ival = val;
	return node;
}

struct Node *new_Boolean(struct Parser *parser, int val, const size_t line) {
	struct Node *node = new_Node_0(parser, N_BOOL, NULL, 0, line);
	node->value.ival = val;
	return node;
}

struct Node *new_String(struct Parser *parser, char *value, size_t len, const size_t line) {
	return new_Node_0(parser, N_STR, value, len, line);
}

DEF_NODE(List, N_LIST, values)
DEF_NODE(Table, N_TABLE, values)

struct Node *new_VariadicContext(struct Node *const expr, const int expected) {
	if (expr->nodetype == N_CALL) {
		expr->value.ival = expected;
		return expr;
	} else if (expr->nodetype == N_MCALL) {
		expr->value.sval.len = expected;
		return expr;
	} else if (expr->nodetype == N_STRINGIFY) {
		new_VariadicContext(Stringify_get_expr(expr), expected);
		return expr;
	} else {
		return expr;
	}
}

void node_del(struct Node *node) {
	if (!node) return;
	node_del(node->next);
	switch (node->nodetype) {
	case N_PATALT:
	case N_BINOP:
	case N_UNOP:
	case N_TRIOP:
	case N_BOOL:
	case N_INT:
	case N_FLOAT:
	case N_PATBOOL:
	case N_PATINT:
	case N_PATFL:
	case N_CALL:
		break;
	case N_VAR:
	case N_LET:
	case N_STR:
	case N_CONST:
	case N_LETITER:
	case N_PATCONST:
	case N_PATLET:
	case N_PATSTR:
		free(node->value.sval.str);
	default:
		break;
	}
	free(node);
}

struct Node *Block_get_block(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_BLOCK, "Expected Block");
	return node->children[0];
}

size_t Body_get_len(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_BODY, "Expected Block");
	return node->children_len;
}

struct Node *Comp_get_expr(const struct Node *const node) {
	return ((node)->children[0]);
}

struct Node *Decl_get_expr(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_LET || node->nodetype == N_CONST, "Expected let or const");
	return ((node)->children[0]);
}

char *Decl_get_name(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_LET || node->nodetype == N_CONST || node->nodetype == N_PATLET || node->nodetype == N_PATCONST, "Expected let or const");
	return node->value.sval.str;
}

struct Node *UnOp_get_expr(const struct Node *const node) {
	return ((node)->value.unop.expr);
}

struct Node *BinOp_get_left(const struct Node *const node) {
	return ((node)->value.binop.left);
}

struct Node *BinOp_get_right(const struct Node *const node) {
	return ((node)->value.binop.right);
}

struct Node *TriOp_get_left(const struct Node *const node) {
	return ((node)->children[0]);
}

struct Node *TriOp_get_middle(const struct Node *const node) {
	return ((node)->children[1]);
}

struct Node *TriOp_get_right(const struct Node *const node) {
	return ((node)->children[2]);
}

struct Node *Comp_get_iter(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_LISTCOMP || node->nodetype == N_TABLECOMP, "Expected Comp");
	return node->children[1];
}

struct Node *Comp_get_cond(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_LISTCOMP || node->nodetype == N_TABLECOMP, "Expected Comp");
	return node->children[2];
}

const char *String_get_str(const struct Node *const node) {
	return node->value.sval.str;
}

size_t String_get_len(const struct Node *const node) {
	return node->value.sval.len;
}

yasl_float Float_get_float(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_FLOAT || node->nodetype == N_PATFL, "Expected Float");
	return node->value.dval;
}

yasl_int Integer_get_int(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_INT || node->nodetype == N_PATINT, "Expected Integer");
	return node->value.ival;
}

bool Boolean_get_bool(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_BOOL || node->nodetype == N_PATBOOL, "Expected Boolean");
	return (bool)node->value.ival;
}
