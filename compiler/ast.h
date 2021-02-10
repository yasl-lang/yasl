#ifndef YASL_AST_H_
#define YASL_AST_H_

#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "yasl_conf.h"

// NOTE: _MUST_ keep this up to date with the jumptable in compiler.c and the jumptable in middleend.c
enum NodeType {
	N_EXPRSTMT,
	N_BLOCK,
	N_BODY,
	N_FNDECL,
	N_RET,
	N_EXPORT,
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
	N_MATCH,
	N_IF,
	N_PRINT,
	N_LET,
	N_CONST,
	N_DECL,
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
	N_TABLE,
	N_ASS,
	N_PATUNDEF,
	N_PATBOOL,
	N_PATFL,
	N_PATINT,
	N_PATSTR,
	N_PATTABLE,
	N_PATLS,
	N_PATVTABLE,
	N_PATVLS,
	N_PATALT,
	N_PATANY,
	N_PATLET,
	N_PATCONST,
};

struct BinOpNode {
	enum Token op;
	struct Node *left;
	struct Node *right;
};

struct UnOpNode {
	enum Token op;
	struct Node *expr;
};

struct TriOpNode {
	struct Node *left;
	struct Node *middle;
	struct Node *right;
};

struct Node1 {
	struct Node *child;
};

struct Node2 {
	struct Node *children[2];
};

struct Node3 {
	struct Node *children[3];
};

struct BodyNode {
	size_t num_children;
	struct Node *children[];
};

/*
struct Node {
	enum NodeType tag;
	void (*visit)(struct Compiler *compiler, struct Node *node);
	union {
		struct UnOpNode unop;
		struct BinOpNode binop;
		struct TriOpNode triop;
	} val;
};
*/

struct Node {
	enum NodeType nodetype;
	union {
		struct {
			char *str;
			size_t str_len;
		} sval;
		yasl_int ival;
		yasl_float dval;
		enum Token type;
		struct UnOpNode unop;
		struct BinOpNode binop;
	} value;
	size_t line;
	// void (*visit)(struct Compiler *compiler, struct Node *node);
	size_t children_len;
	struct Node *children[];
};

void body_append(struct Node **node, struct Node *const child);

struct Node *node_clone(const struct Node *const node);

#define FOR_CHILDREN(i, child, node) struct Node *child;\
for (size_t i = 0; i < (node)->children_len; i++ ) if (child = (node)->children[i], child != NULL)

struct Node *Assert_get_expr(const struct Node *const node);
struct Node *Const_get_expr(const struct Node *const node);
struct Node *If_get_cond(const struct Node *const node);
struct Node *If_get_then(const struct Node *const node);
struct Node *If_get_else(const struct Node *const node);
struct Node *Block_get_block(const struct Node *const node);
size_t Body_get_len(const struct Node *const node);
struct Node *Comp_get_expr(const struct Node *const node);
struct Node *ForIter_get_iter(const struct Node *const node);
struct Node *ForIter_get_body(const struct Node *const node);
struct Node *While_get_cond(const struct Node *const node);
struct Node *While_get_body(const struct Node *const node);
struct Node *While_get_post(const struct Node *const node);
struct Node *Table_get_values(const struct Node *const node);
struct Node *While_get_body(const struct Node *const node);
struct Node *Match_get_expr(const struct Node *const node);
struct Node *Match_get_patterns(const struct Node *const node);
struct Node *Match_get_guards(const struct Node *const node);
struct Node *Match_get_bodies(const struct Node *const node);
struct Node *Print_get_expr(const struct Node *const node);
struct Node *Decl_get_expr(const struct Node *const node);
char *Decl_get_name(const struct Node *const node);
struct Node *List_get_values(const struct Node *const node);
struct Node *ExprStmt_get_expr(const struct Node *const node);
struct Node *FnDecl_get_params(const struct Node *const node);
struct Node *FnDecl_get_body(const struct Node *const node);
char *MCall_get_name(const struct Node *const node);
struct Node *Call_get_params(const struct Node *const node);
struct Node *Call_get_object(const struct Node *const node);
struct Node *Return_get_expr(const struct Node *const node);
struct Node *Export_get_expr(const struct Node *const node);
struct Node *Set_get_collection(const struct Node *const node);
struct Node *Set_get_key(const struct Node *const node);
struct Node *Set_get_value(const struct Node *const node);
struct Node *Get_get_collection(const struct Node *const node);
struct Node *Get_get_value(const struct Node *const node);
struct Node *Slice_get_collection(const struct Node *const node);
struct Node *Slice_get_start(const struct Node *const node);
struct Node *Slice_get_end(const struct Node *const node);
struct Node *UnOp_get_expr(const struct Node *const node);
struct Node *BinOp_get_left(const struct Node *const node);
struct Node *BinOp_get_right(const struct Node *const node);
struct Node *TriOp_get_left(const struct Node *const node);
struct Node *TriOp_get_middle(const struct Node *const node);
struct Node *TriOp_get_right(const struct Node *const node);
struct Node *Assign_get_expr(const struct Node *const node);
struct Node *Comp_get_iter(const struct Node *const node);
struct Node *Comp_get_cond(const struct Node *const node);
struct Node *LetIter_get_var(const struct Node *const node);
struct Node *LetIter_get_collection(const struct Node *const node);
const char *String_get_str(const struct Node *const node);
size_t String_get_len(const struct Node *const node);
yasl_int Integer_get_int(const struct Node *const node);
yasl_float Float_get_float(const struct Node *const node);
bool Boolean_get_bool(const struct Node *const node);
const char *Var_get_name(const struct Node *const node);

struct Node *new_ExprStmt(const struct Node *const child, const size_t line);
struct Node *new_Block(const struct Node *const body, const size_t line);
struct Node *new_Body(const size_t line);
struct Node *new_FnDecl(const struct Node *const params, const struct Node *const body, char *name, size_t name_len, const size_t line);
struct Node *new_Return(struct Node *expr, const size_t line);
struct Node *new_Export(struct Node *expr, const size_t line);
struct Node *new_Set(struct Node *collection, struct Node *key, struct Node *value, const size_t line);
struct Node *new_Get(struct Node *collection, struct Node *value, const size_t line);
struct Node *new_Slice(struct Node *collection, struct Node *start, struct Node *end, const size_t line);
struct Node *new_Call(struct Node *params, struct Node *object, const size_t line);
struct Node *new_MethodCall(struct Node *params, struct Node *object, char *value, size_t len, const size_t line);
struct Node *new_LetIter(char *const name, struct Node *collection, const size_t line);
struct Node *new_ListComp(struct Node *expr, struct Node *iter, struct Node *cond, const size_t line);
struct Node *new_TableComp(struct Node *expr, struct Node *iter, struct Node *cond, const size_t line);
struct Node *new_ForIter(struct Node *iter, struct Node *body, const size_t line);
struct Node *new_While(struct Node *cond, struct Node *body, struct Node *post, const size_t line);
struct Node *new_Break(size_t line);
struct Node *new_Continue(size_t line);
struct Node *new_Match(struct Node *cond, struct Node *pats, struct Node *guards, struct Node *bodies, const size_t line);
struct Node *new_If(struct Node *cond, struct Node *then_node, struct Node *else_node, const size_t line);
struct Node *new_Print(struct Node *expr, const size_t line);
struct Node *new_Assert(struct Node *expr, const size_t line);
struct Node *new_Let(char *const name, struct Node *expr, const size_t line);
struct Node *new_Const(char *const name, struct Node *expr, const size_t line);
struct Node *new_TriOp(enum Token op, struct Node *left, struct Node *middle, struct Node *right, const size_t line);
struct Node *new_BinOp(enum Token op, struct Node *left, struct Node *right, const size_t line);
struct Node *new_UnOp(enum Token op, struct Node *child, const size_t line);
struct Node *new_Assign(char *const name, struct Node *child, const size_t line);
struct Node *new_Var(char *const name, const size_t line);
struct Node *new_Undef(size_t line);
struct Node *new_Float(yasl_float val, const size_t line);
struct Node *new_Integer(yasl_int val, const size_t line);
struct Node *new_Boolean(int value, const size_t line);
struct Node *new_String(char *value, size_t len, const size_t line);
struct Node *new_List(struct Node *values, const size_t line);
struct Node *new_Table(struct Node *keys, const size_t line);

void node_del(struct Node *node);

#endif
