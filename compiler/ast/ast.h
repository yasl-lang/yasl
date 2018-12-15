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

struct Node_s {
    AST nodetype;
    Token type;
    struct Node_s **children;
    size_t children_len;
    char* name;
    size_t name_len;
    size_t line;
};

typedef struct Node_s Node;

void body_append(Node *const node, Node *const child);

Node *node_clone(const Node *const node);

Node *ExprStmt_get_expr(Node *node);
Node *new_ExprStmt(Node *child, size_t line);
Node *Block_get_body(Node *node);
Node *new_Block(Node *body, size_t line);
Node *new_Body(size_t line);
Node *FnDecl_get_params(Node *node);
Node *FnDecl_get_body(Node *node);
Node *new_FnDecl(Node *params, Node *body, char *name, int64_t name_len, size_t line);
Node *Return_get_expr(Node *node);
Node *new_Return(Node *expr, size_t line);
Node *Set_get_collection(Node *node);
Node *Set_get_key(Node *node);
Node *Set_get_value(Node *node);
Node *new_Set(Node *collection, Node *key, Node *value, size_t line);
Node *Get_get_collection(Node *node);
Node *Get_get_value(Node *node);
Node *new_Get(Node *collection, Node *value, size_t line);
Node *Call_get_params(Node *node);
Node *Call_get_object(Node *node);
Node *new_Call(Node *params, Node *object, size_t line);
Node *new_LetIter(Node *var, Node *collection, size_t line);
Node *new_Iter(Node *var, Node *collection, size_t line);
Node *ListComp_get_expr(const Node *const node);
Node *ListComp_get_var(const Node *const node);
Node *ListComp_get_collection(const Node *const node);
Node *new_ListComp(Node *expr, Node *iter, Node *cond, size_t line);
Node *TableComp_get_key_value(const Node *const node);
Node *new_TableComp(Node *expr, Node *iter, Node *cond, size_t line);
Node *ForIter_get_body(const Node *const node);
Node *new_ForIter(Node *iter, Node *body, size_t line);
Node *While_get_cond(const Node *const node);
Node *While_get_body(const Node *const node);
Node *new_While(Node *cond, Node *body, Node *post, size_t line);
Node *new_Break(size_t line);
Node *new_Continue(size_t line);
Node *If_get_cond(Node *node);
Node *If_get_then(Node *node);
Node *If_get_else(Node *node);
Node *new_If(Node *cond, Node *then_node, Node *else_node, size_t line);
Node *Print_get_expr(const Node *const node);
Node *new_Print(Node *expr, size_t line);
Node *Let_get_expr(const Node *const node);
Node *new_Let(char *name, int64_t name_len, Node *expr, size_t line);
Node *Const_get_expr(Node *node);
Node *new_Const(char *name, int64_t name_len, Node *expr, size_t line);
Node *TriOp_get_left(Node *node);
Node *TriOp_get_middle(Node *node);
Node *TriOp_get_right(Node *node);
Node *new_TriOp(Token op, Node *left, Node *middle, Node *right, size_t line);
Node *BinOp_get_left(Node *node);
Node *BinOp_get_right(Node *node);
Node *new_BinOp(Token op, Node *left, Node *right, size_t line);
Node *UnOp_get_expr(Node *node);
Node *new_UnOp(Token op, Node *child, size_t line);
Node *Assign_get_expr(Node *node);
Node *new_Assign(char *name, int64_t name_len, Node *child, size_t line);
Node *new_Var(char *name, int64_t name_len, size_t line);
Node *new_Undef(size_t line);
Node *new_Float(char *value, size_t len, size_t line);
Node *new_Integer(char *value, size_t len, size_t line);
Node *new_Boolean(char *value, size_t len, size_t line);
Node *new_String(char *value, size_t len, size_t line);
Node *List_get_values(const Node *const node);
Node *new_List(Node *values, size_t line);
Node *Table_get_values(const Node *const node);
Node *new_Table(Node *keys, size_t line);

void node_del(Node *node);
