#include "ast.h"

#include <stdarg.h>

#include "debug.h"
#include "yasl_conf.h"

struct Node *node_clone(const struct Node *const node) {
	if (node == NULL) return NULL;
	struct Node *clone = (struct Node *)malloc(sizeof(struct Node) + node->children_len * sizeof(struct Node *));
	clone->nodetype = node->nodetype;
	// clone->type = node->type;
	clone->children_len = node->children_len;
	for (size_t i = 0; i < clone->children_len; i++) {
		clone->children[i] = node_clone(node->children[i]);
	}

	switch (node->nodetype) {
	case N_BINOP:
	case N_UNOP:
	case N_TRIOP:
		clone->value.type = node->value.type;
		break;
	case N_INT:
	case N_BOOL:
	case N_FLOAT: clone->value.ival = node->value.ival;
		break;
	default: clone->value.sval.str_len = node->value.sval.str_len;
      	  clone->value.sval.str = (char *)malloc(node->value.sval.str_len);
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

struct Node *new_ExprStmt(const struct Node *const child, const size_t line) {
	return new_Node_1(N_EXPRSTMT, child, NULL, 0, line);
}

struct Node *new_Block(const struct Node *const body, const size_t line) {
	return new_Node_1(N_BLOCK, body, NULL, 0, line);
}

struct Node *new_Body(const size_t line) {
	return new_Node_0(N_BODY, NULL, 0, line);
}

void body_append(struct Node **node, struct Node *const child) {
	YASL_COMPILE_DEBUG_LOG("%s\n", "appending to block");
	*node = (struct Node*)realloc(*node, sizeof(struct Node) + (++(*node)->children_len) * sizeof(struct Node *));
	(*node)->children[(*node)->children_len - 1] = child;
}

struct Node *new_FnDecl(const struct Node *const params, const struct Node *const body, char *name, size_t name_len, const size_t line) {
	return new_Node_2(N_FNDECL, params, body, name, name_len, line);
}

struct Node *new_Return(struct Node *expr, const size_t line) {
	return new_Node_1(N_RET, expr, NULL, 0, line);
}

struct Node *new_Export(struct Node *expr, const size_t line) {
	return new_Node_1(N_EXPORT, expr, NULL, 0, line);
}


struct Node *new_Call(struct Node *params, struct Node *object, const size_t line) {
	return new_Node_2(N_CALL, params, object, NULL, 0, line);
}

struct Node *new_MethodCall(struct Node *params, struct Node *object, char *value, size_t len, const size_t line) {
	return new_Node_2(N_MCALL, params, object, value, len, line);
}

struct Node *new_Set(struct Node *collection, struct Node *key, struct Node *value, const size_t line) {
	return new_Node_3(N_SET, collection, key, value, NULL, 0, line);
}

struct Node *new_Get(struct Node *collection, struct Node *value, const size_t line) {
	return new_Node_2(N_GET, collection, value, NULL, 0, line);
}

struct Node *new_Slice(struct Node *collection, struct Node *start, struct Node *end, const size_t line) {
	return new_Node_3(N_SLICE, collection, start, end, NULL, 0, line);
}

struct Node *new_ListComp(struct Node *expr, struct Node *iter, struct Node *cond, const size_t line) {
	return new_Node_3(N_LISTCOMP, expr, iter, cond, NULL, 0, line);
}

struct Node *new_TableComp(struct Node *expr, struct Node *iter, struct Node *cond, const size_t line) {
	return new_Node_3(N_TABLECOMP, expr, iter, cond, NULL, 0, line);
}

struct Node *new_LetIter(struct Node *var, struct Node *collection, const size_t line) {
	return new_Node_2(N_LETITER, var, collection, NULL, 0, line);
}

struct Node *new_ForIter(struct Node *iter, struct Node *body, const size_t line) {
	return new_Node_2(N_FORITER, iter, body, NULL, 0, line);
}

struct Node *new_While(struct Node *cond, struct Node *body, struct Node *post, const size_t line) {
	return new_Node_3(N_WHILE, cond, body, post, NULL, 0, line);
}

struct Node *new_Break(size_t line) {
	return new_Node_0(N_BREAK, NULL, 0, line);
}

struct Node *new_Continue(size_t line) {
	return new_Node_0(N_CONT, NULL, 0, line);
}

struct Node *new_If(struct Node *cond, struct Node *then_node, struct Node *else_node, const size_t line) {
	return new_Node_3(N_IF, cond, then_node, else_node, NULL, 0, line);
}

struct Node *new_Print(struct Node *expr, const size_t line) {
	return new_Node_1(N_PRINT, expr, NULL, 0, line);
}

struct Node *new_Let(char *name, size_t name_len, struct Node *expr, const size_t line) {
	return new_Node_1(N_LET, expr, name, name_len, line);
}

struct Node *new_Const(char *name, size_t name_len, struct Node *expr, const size_t line) {
	return new_Node_1(N_CONST, expr, name, name_len, line);
}

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
	node->children_len = 2;
	node->value.type = op;
	node->line = line;
	node->children[0] = left;
	node->children[1] = right;
	return node;
}

struct Node *new_UnOp(enum Token op, struct Node *child, const size_t line) {
	struct Node *const node = (struct Node *)malloc(sizeof(struct Node) + sizeof(struct Node *) * 1);
	node->nodetype = N_UNOP;
	node->children_len = 1;
	node->value.type = op;
	node->line = line;
	node->children[0] = child;
	return node;
}

struct Node *new_Assign(char *name, size_t name_len, struct Node *child, const size_t line) {
	return new_Node_1(N_ASSIGN, child, name, name_len, line);
}

struct Node *new_Var(char *name, size_t name_len, const size_t line) {
	return new_Node_0(N_VAR, name, name_len, line);
}

struct Node *new_Undef(size_t line) {
	return new_Node_0(N_UNDEF, NULL, 0, line);
}

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

struct Node *new_List(struct Node *values, const size_t line) {
	return new_Node_1(N_LIST, values, NULL, 0, line);
}

struct Node *new_Table(struct Node *keys, const size_t line) {
	return new_Node_1(N_TABLE, keys, NULL, 0, line);
}

void node_del(struct Node *node) {
	if (!node) return;
	while (node->children_len-- > 0) {
		if (node->children[node->children_len] != NULL)
			node_del(node->children[node->children_len]);
	}
	switch (node->nodetype) {
	case N_BINOP:
	case N_UNOP:
	case N_TRIOP:
	case N_BOOL:
	case N_INT:
	case N_FLOAT:
		break;
	default:
		free(node->value.sval.str);
	}
	free(node);
}
