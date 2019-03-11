#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/token.h"
#include "yasl_conf.h"

// NOTE: _MUST_ keep this up to date with the jumptable in compiler.c and the jumptable in middleend.c
typedef enum NodeType {
	N_EXPRSTMT,
	N_BLOCK,
	N_BODY,
	N_FNDECL,
	N_RET,
	N_CALL,
	N_MCALL,
	N_SET,
	N_GET,
	N_SLICE,
	N_LETITER,
	N_LISTCOMP,
	N_TABLECOMP,
	N_FORITER,
	N_WHILE,
	N_BREAK,
	N_CONT,
	N_IF,
	N_PRINT,
	N_LET,
	N_CONST,
	N_TRIOP,
	N_BINOP,
	N_UNOP,
	N_ASSIGN,
	N_VAR,
	N_UNDEF,
	N_FLOAT,
	N_INT,
	N_BOOL,
	N_STR,
	N_LIST,
	N_TABLE
} AST;

struct Node {
	AST nodetype;
	enum Token type;
	size_t line;
	union {
		struct {
			char *str;
			size_t str_len;
		} sval;
		yasl_int ival;
		yasl_float dval;
	} value;
	size_t children_len;
	struct Node *children[];
};

void body_append(struct Node **node, struct Node *const child);

struct Node *node_clone(const struct Node *const node);

#define FOR_CHILDREN(i, child, node) struct Node *child;\
for (size_t i = 0; i < (node)->children_len; i++ ) if (child = (node)->children[i], child != NULL)

#define Const_get_expr(node) ((node)->children[0])
#define TableComp_get_key_value(node) ((node)->children[0])
#define ListComp_get_expr(node) ((node)->children[0])
#define ForIter_get_body(node) ((node)->children[1])
#define While_get_cond(node) ((node)->children[0])
#define Table_get_values(node) ((node)->children[0])
#define While_get_body(node) ((node)->children[1])
#define Print_get_expr(node) ((node)->children[0])
#define Let_get_expr(node) ((node)->children[0])
#define List_get_values(node) ((node)->children[0])
#define ExprStmt_get_expr(node) ((node)->children[0])
#define FnDecl_get_params(node) ((node)->children[0])
#define FnDecl_get_body(node) ((node)->children[1])
#define Call_get_params(node) ((node)->children[0])
#define Return_get_expr(node) ((node)->children[0])
#define Set_get_collection(node) ((node)->children[0])
#define Set_get_key(node) ((node)->children[1])
#define Set_get_value(node) ((node)->children[2])
#define Get_get_collection(node) ((node)->children[0])
#define Get_get_value(node) ((node)->children[1])
#define Slice_get_collection(node) ((node)->children[0])
#define Slice_get_start(node) ((node)->children[1])
#define Slice_get_end(node) ((node)->children[2])
#define UnOp_get_expr(node) ((node)->children[0])
#define Assign_get_expr(node) ((node)->children[0])



struct Node *new_ExprStmt(struct Node *child, size_t line);
struct Node *new_Block(struct Node *body, size_t line);
struct Node *new_Body(size_t line);
struct Node *new_FnDecl(struct Node *params, struct Node *body, char *name, size_t name_len, size_t line);
struct Node *new_Return(struct Node *expr, size_t line);
struct Node *new_Set(struct Node *collection, struct Node *key, struct Node *value, size_t line);
struct Node *new_Get(struct Node *collection, struct Node *value, size_t line);
struct Node *new_Slice(struct Node *collection, struct Node *start, struct Node *end, size_t line);
struct Node *new_Call(struct Node *params, struct Node *object, size_t line);
struct Node *new_MethodCall(struct Node *params, struct Node *object, char *value, size_t len, size_t line);
struct Node *new_LetIter(struct Node *var, struct Node *collection, size_t line);
struct Node *new_ListComp(struct Node *expr, struct Node *iter, struct Node *cond, size_t line);
struct Node *new_TableComp(struct Node *expr, struct Node *iter, struct Node *cond, size_t line);
struct Node *new_ForIter(struct Node *iter, struct Node *body, size_t line);
struct Node *new_While(struct Node *cond, struct Node *body, struct Node *post, size_t line);
struct Node *new_Break(size_t line);
struct Node *new_Continue(size_t line);
struct Node *new_If(struct Node *cond, struct Node *then_node, struct Node *else_node, size_t line);
struct Node *new_Print(struct Node *expr, size_t line);
struct Node *new_Let(char *name, size_t name_len, struct Node *expr, size_t line);
struct Node *new_Const(char *name, size_t name_len, struct Node *expr, size_t line);
struct Node *new_TriOp(enum Token op, struct Node *left, struct Node *middle, struct Node *right, size_t line);
struct Node *new_BinOp(enum Token op, struct Node *left, struct Node *right, size_t line);
struct Node *new_UnOp(enum Token op, struct Node *child, size_t line);
struct Node *new_Assign(char *name, size_t name_len, struct Node *child, size_t line);
struct Node *new_Var(char *name, size_t name_len, size_t line);
struct Node *new_Undef(size_t line);
struct Node *new_Float(double val, size_t line);
struct Node *new_Integer(yasl_int val, size_t line);
struct Node *new_Boolean(int value, size_t line);
struct Node *new_String(char *value, size_t len, size_t line);
struct Node *new_List(struct Node *values, size_t line);
struct Node *new_Table(struct Node *keys, size_t line);

void node_del(struct Node *node);
