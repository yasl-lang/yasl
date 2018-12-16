#pragma once

#include "../token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: must keep this up to date with the jumptable in compiler.c
typedef enum {
    N_EXPRSTMT,
    N_BLOCK,
    N_BODY,
    N_FNDECL,
    N_RET,
    N_CALL,
    N_SET,
    N_GET,
    N_LETITER,
    N_ITER,
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
    N_FLOAT64,
    N_INT64,
    N_BOOL,
    N_STR,
    N_LIST,
    N_TABLE
} AST;

struct Node {
    AST nodetype;
    Token type;
    char* name;
    size_t name_len;
    size_t line;
    size_t children_len;
    struct Node *children[];
};

void body_append(struct Node **node, struct Node *const child);

struct Node *node_clone(const struct Node *const node);

struct Node *ExprStmt_get_expr(struct Node *node);
struct Node *new_ExprStmt(struct Node *child, size_t line);
struct Node *Block_get_body(struct Node *node);
struct Node *new_Block(struct Node *body, size_t line);
struct Node *new_Body(size_t line);
struct Node *FnDecl_get_params(struct Node *node);
struct Node *FnDecl_get_body(struct Node *node);
struct Node *new_FnDecl(struct Node *params, struct Node *body, char *name, int64_t name_len, size_t line);
struct Node *Return_get_expr(struct Node *node);
struct Node *new_Return(struct Node *expr, size_t line);
struct Node *Set_get_collection(struct Node *node);
struct Node *Set_get_key(struct Node *node);
struct Node *Set_get_value(struct Node *node);
struct Node *new_Set(struct Node *collection, struct Node *key, struct Node *value, size_t line);
struct Node *Get_get_collection(struct Node *node);
struct Node *Get_get_value(struct Node *node);
struct Node *new_Get(struct Node *collection, struct Node *value, size_t line);
struct Node *Call_get_params(struct Node *node);
struct Node *Call_get_object(struct Node *node);
struct Node *new_Call(struct Node *params, struct Node *object, size_t line);
struct Node *new_LetIter(struct Node *var, struct Node *collection, size_t line);
struct Node *new_Iter(struct Node *var, struct Node *collection, size_t line);
struct Node *ListComp_get_expr(const struct Node *const node);
struct Node *ListComp_get_var(const struct Node *const node);
struct Node *ListComp_get_collection(const struct Node *const node);
struct Node *new_ListComp(struct Node *expr, struct Node *iter, struct Node *cond, size_t line);
struct Node *TableComp_get_key_value(const struct Node *const node);
struct Node *new_TableComp(struct Node *expr, struct Node *iter, struct Node *cond, size_t line);
struct Node *ForIter_get_body(const struct Node *const node);
struct Node *new_ForIter(struct Node *iter, struct Node *body, size_t line);
struct Node *While_get_cond(const struct Node *const node);
struct Node *While_get_body(const struct Node *const node);
struct Node *new_While(struct Node *cond, struct Node *body, struct Node *post, size_t line);
struct Node *new_Break(size_t line);
struct Node *new_Continue(size_t line);
struct Node *If_get_cond(struct Node *node);
struct Node *If_get_then(struct Node *node);
struct Node *If_get_else(struct Node *node);
struct Node *new_If(struct Node *cond, struct Node *then_node, struct Node *else_node, size_t line);
struct Node *Print_get_expr(const struct Node *const node);
struct Node *new_Print(struct Node *expr, size_t line);
struct Node *Let_get_expr(const struct Node *const node);
struct Node *new_Let(char *name, int64_t name_len, struct Node *expr, size_t line);
struct Node *Const_get_expr(struct Node *node);
struct Node *new_Const(char *name, int64_t name_len, struct Node *expr, size_t line);
struct Node *TriOp_get_left(struct Node *node);
struct Node *TriOp_get_middle(struct Node *node);
struct Node *TriOp_get_right(struct Node *node);
struct Node *new_TriOp(Token op, struct Node *left, struct Node *middle, struct Node *right, size_t line);
struct Node *BinOp_get_left(struct Node *node);
struct Node *BinOp_get_right(struct Node *node);
struct Node *new_BinOp(Token op, struct Node *left, struct Node *right, size_t line);
struct Node *UnOp_get_expr(struct Node *node);
struct Node *new_UnOp(Token op, struct Node *child, size_t line);
struct Node *Assign_get_expr(struct Node *node);
struct Node *new_Assign(char *name, int64_t name_len, struct Node *child, size_t line);
struct Node *new_Var(char *name, int64_t name_len, size_t line);
struct Node *new_Undef(size_t line);
struct Node *new_Float(char *value, size_t len, size_t line);
struct Node *new_Integer(char *value, size_t len, size_t line);
struct Node *new_Boolean(char *value, size_t len, size_t line);
struct Node *new_String(char *value, size_t len, size_t line);
struct Node *List_get_values(const struct Node *const node);
struct Node *new_List(struct Node *values, size_t line);
struct Node *Table_get_values(const struct Node *const node);
struct Node *new_Table(struct Node *keys, size_t line);

void node_del(struct Node *node);
