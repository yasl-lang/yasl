#include "ast.h"

#include <stdarg.h>

#include "debug.h"
#include "yasl_conf.h"

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
		clone->value.ival = node->value.ival;
		break;
	default:
		clone->value.sval.str_len = node->value.sval.str_len;
		clone->value.sval.str = (char *) malloc(node->value.sval.str_len + 1);
		clone->value.sval.str[clone->value.sval.str_len] = '\0';
		memcpy(clone->value.sval.str, node->value.sval.str, clone->value.sval.str_len);
	}

	clone->line = node->line;
	return clone;
}

static struct Node *new_Node(const enum NodeType nodetype, const size_t line, const size_t name_len,
		char *const name /* OWN */, const size_t n, ... /* OWN */) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * n);
	node->nodetype = nodetype;
	node->children_len = n;

	node->value.sval.str_len = name_len;
	node->value.sval.str = name;

	node->line = line;
	va_list children;
	va_start(children, n);
	for (size_t i = 0; i < n; i++) {
		node->children[i] = va_arg(children, struct Node*);
	}
	va_end(children);
	return node;
}

#define new_Node_0(nodetype, name, name_len, line) new_Node(nodetype, line, name_len, name, 0)
#define new_Node_1(nodetype, child, name, name_len, line) new_Node(nodetype, line, name_len, name, 1, child)
#define new_Node_2(nodetype, child1, child2, name, name_len, line) new_Node(nodetype, line, name_len, name, 2, child1, child2)
#define new_Node_3(nodetype, child1, child2, child3, name, name_len, line) new_Node(nodetype, line, name_len, name, 3, child1, child2, child3)
#define new_Node_4(nodetype, child1, child2, child3, child4, name, name_len, line) new_Node(nodetype, line, name_len, name, 4, child1, child2, child3, child4)

#define DEF_NODE(...) GET_MACRO(__VA_ARGS__, DEF_NODE4, DEF_NODE3, DEF_NODE2, DEF_NODE1, DEF_NODE0)(__VA_ARGS__)
#define DEF_STR_NODE(...) GET_MACRO(__VA_ARGS__, DEF_STR_NODE4, DEF_STR_NODE3, DEF_STR_NODE2, DEF_STR_NODE1, DEF_STR_NODE0)(__VA_ARGS__)
#define DEF_ZSTR_NODE(...) GET_MACRO(__VA_ARGS__, DEF_ZSTR_NODE4, DEF_ZSTR_NODE3, DEF_ZSTR_NODE2, DEF_ZSTR_NODE1, DEF_ZSTR_NODE0)(__VA_ARGS__)
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
struct Node *new_##name(size_t line) {\
	return new_Node_0(E, NULL, 0, line);\
}

#define DEF_NODE1(name, E, a) \
struct Node *new_##name(const struct Node *const a, const size_t line) {\
	return new_Node_1(E, a, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)

#define DEF_NODE2(name, E, a, b) \
struct Node *new_##name(const struct Node *const a, const struct Node *const b, const size_t line) {\
	return new_Node_2(E, a, b, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)

#define DEF_NODE3(name, E, a, b, c) \
struct Node *new_##name(const struct Node *const a, const struct Node *const b, const struct Node *const c, const size_t line) {\
	return new_Node_3(E, a, b, c, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)\
DEF_GETTER(name, E, c, 2)

#define DEF_NODE4(name, E, a, b, c, d) \
struct Node *new_##name(const struct Node *const a, const struct Node *const b, const struct Node *const c, const struct Node *const d, const size_t line) {\
	return new_Node_4(E, a, b, c, d, NULL, 0, line);\
}\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)\
DEF_GETTER(name, E, c, 2)\
DEF_GETTER(name, E, d, 3)

#define DEF_STR_NODE2(name, E, a, b) \
struct Node *new_##name(const struct Node *const a, const struct Node *const b, char *name, size_t name_len, const size_t line) {\
	return new_Node_2(E, a, b, name, name_len, line);\
}\
DEF_GETNAME(name, E)\
DEF_GETTER(name, E, a, 0)\
DEF_GETTER(name, E, b, 1)

#define DEF_ZSTR_NODE0(name, E) \
struct Node *new_##name(char *name, const size_t line) {\
	return new_Node_0(E, name, strlen(name), line);\
}\
DEF_GETNAME(name, E)

#define DEF_ZSTR_NODE1(name, E, a) \
struct Node *new_##name(const struct Node *const a, char *name, const size_t line) {\
	return new_Node_1(E, a, name, strlen(name), line);\
}\
DEF_GETNAME(name, E)\
DEF_GETTER(name, E, a, 0)


void body_append(struct Node **node, struct Node *const child) {
	YASL_COMPILE_DEBUG_LOG("%s\n", "appending to block");
	*node = (struct Node*)realloc(*node, sizeof(struct Node) + (++(*node)->children_len) * sizeof(struct Node *));
	(*node)->children[(*node)->children_len - 1] = child;
}

DEF_NODE(ExprStmt, N_EXPRSTMT, expr)

struct Node *new_Block(const struct Node *const body, const size_t line) {
	return new_Node_1(N_BLOCK, body, NULL, 0, line);
}

struct Node *new_Body(const size_t line) {
	return new_Node_0(N_BODY, NULL, 0, line);
}

DEF_NODE(Decl, N_DECL, lvals, rvals)
DEF_STR_NODE(FnDecl, N_FNDECL, params, body)
DEF_NODE(Return, N_RET, expr)
DEF_NODE(MultiReturn, N_MULTIRET, expres)
DEF_NODE(Export, N_EXPORT, expr)
DEF_NODE(Call, N_CALL, params, object)
DEF_STR_NODE(MethodCall, N_MCALL, params, object)
DEF_NODE(Set, N_SET, collection, key, value)
DEF_NODE(Get, N_GET, collection, value)
DEF_NODE(Slice, N_SLICE, collection, start, end)
DEF_NODE(ListComp, N_LISTCOMP, expr, iter, cond)
DEF_NODE(TableComp, N_TABLECOMP, expr, iter, cond)
DEF_ZSTR_NODE(LetIter, N_LETITER, collection)
DEF_NODE(ForIter, N_FORITER, iter, body)
DEF_NODE(While, N_WHILE, cond, body, post)
DEF_NODE(Break, N_BREAK)
DEF_NODE(Continue, N_CONT)
DEF_NODE(Match, N_MATCH, cond, patterns, guards, bodies)
DEF_NODE(If, N_IF, cond, then, el)
DEF_NODE(Print, N_PRINT, expr)
DEF_NODE(Assert, N_ASS, expr)
DEF_ZSTR_NODE(Let, N_LET, expr)
DEF_ZSTR_NODE(Const, N_CONST, expr)

struct Node *new_TriOp(enum Token op, struct Node *left, struct Node *middle, struct Node *right, const size_t line) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * 3);
	node->nodetype = N_TRIOP;
	node->children_len = 3;
	node->value.type = op;
	node->line = line;
	node->children[0] = left;
	node->children[1] = middle;
	node->children[2] = right;
	return node;
}

struct Node *new_BinOp(enum Token op, struct Node *left, struct Node *right, const size_t line) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * 2);
	node->nodetype = N_BINOP;
	node->children_len = 0;
	node->value.binop = ((struct BinOpNode) { op, left, right });
	node->line = line;
	return node;
}

struct Node *new_UnOp(enum Token op, struct Node *child, const size_t line) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * 1);
	node->nodetype = N_UNOP;
	node->children_len = 0;
	node->value.unop = ((struct UnOpNode) { op, child });
	node->line = line;
	return node;
}

DEF_ZSTR_NODE(Assign, N_ASSIGN, expr)
DEF_ZSTR_NODE(Var, N_VAR)
DEF_NODE(Undef, N_UNDEF)

struct Node *new_Float(yasl_float val, const size_t line) {
	struct Node *node = new_Node_0(N_FLOAT, NULL, 0, line);
	node->value.dval = val;
	return node;
}

struct Node *new_Integer(yasl_int val, const size_t line) {
	struct Node *node = new_Node_0(N_INT, NULL, 0, line);
	node->value.ival = val;
	return node;
}

struct Node *new_Boolean(int val, const size_t line) {
	struct Node *node = new_Node_0(N_BOOL, NULL, 0, line);
	node->value.ival = val;
	return node;
}

struct Node *new_String(char *value, size_t len, const size_t line) {
	return new_Node_0(N_STR, value, len, line);
}

DEF_NODE(List, N_LIST, values)
DEF_NODE(Table, N_TABLE, values)

void node_del(struct Node *node) {
	if (!node) return;
	while (node->children_len-- > 0) {
		if (node->children[node->children_len] != NULL)
			node_del(node->children[node->children_len]);
	}
	switch (node->nodetype) {
	case N_PATALT:
	case N_BINOP:
		node_del(node->value.binop.left);
		node_del(node->value.binop.right);
		break;
	case N_UNOP:
		node_del(node->value.unop.expr);
		break;
	case N_TRIOP:
	case N_BOOL:
	case N_INT:
	case N_FLOAT:
	case N_PATBOOL:
	case N_PATINT:
	case N_PATFL:
		break;
	default:
		free(node->value.sval.str);
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
	return node->value.sval.str_len;
}

yasl_float Float_get_float(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_FLOAT, "Expected Float");
	return node->value.dval;
}

yasl_int Integer_get_int(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_INT, "Expected Integer");
	return node->value.ival;
}

bool Boolean_get_bool(const struct Node *const node) {
	YASL_ASSERT(node->nodetype == N_BOOL, "Expected Boolean");
	return (bool)node->value.ival;
}
