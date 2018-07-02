#pragma once

#include "../token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    N_EXPRSTMT,
    N_BLOCK,
    N_BODY,
    N_FNDECL,
    N_RET,
    N_CALL,
    N_SET,
    N_GET,
    N_WHILE,
    N_BREAK,
    N_CONT,
    N_IF,
    N_PRINT,
    N_LET,
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
    int64_t children_len;
    char* name;
    int64_t name_len;
    int line;
};

typedef struct Node_s Node;

void body_append(Node *const node, Node *const child);

Node *node_clone(const Node *const node);

Node *ExprStmt_get_expr(Node *node);
Node *new_ExprStmt(Node *child, int line);
Node *Block_get_body(Node *node);
Node *new_Block(Node *body, int line);
Node *new_Body(int line);
Node *FnDecl_get_params(Node *node);
Node *FnDecl_get_body(Node *node);
Node *new_FnDecl(Node *params, Node *body, char *name, int64_t name_len, int line);
Node *Return_get_expr(Node *node);
Node *new_Return(Node *expr, int line);
Node *Set_get_collection(Node *node);
Node *Set_get_key(Node *node);
Node *Set_get_value(Node *node);
Node *new_Set(Node *collection, Node *key, Node *value, int line);
Node *Get_get_collection(Node *node);
Node *Get_get_value(Node *node);
Node *new_Get(Node *collection, Node *value, int line);
Node *Call_get_params(Node *node);
Node *Call_get_object(Node *node);
Node *new_Call(Node *params, Node *object, int line);
Node *While_get_cond(Node *node);
Node *While_get_body(Node *node);
Node *new_While(Node *cond, Node *body, int line);
Node *new_Break(int line);
Node *new_Continue(int line);
Node *If_get_cond(Node *node);
Node *If_get_then(Node *node);
Node *If_get_else(Node *node);
Node *new_If(Node *cond, Node *then_node, Node *else_node, int line);
Node *Print_get_expr(Node *node);
Node *new_Print(Node *expr, int line);
Node *Let_get_expr(Node *node);
Node *new_Let(char *name, int64_t name_len, Node *expr, int line);
Node *TriOp_get_left(Node *node);
Node *TriOp_get_middle(Node *node);
Node *TriOp_get_right(Node *node);
Node *new_TriOp(Token op, Node *left, Node *middle, Node *right, int line);
Node *BinOp_get_left(Node *node);
Node *BinOp_get_right(Node *node);
Node *new_BinOp(Token op, Node *left, Node *right, int line);
Node *UnOp_get_expr(Node *node);
Node *new_UnOp(Token op, Node *child, int line);
Node *Assign_get_expr(Node *node);
Node *new_Assign(char *name, int64_t name_len, Node *child, int line);
Node *new_Var(char *name, int64_t name_len, int line);
Node *new_Undef(int line);
Node *new_Float(char *value, int len, int line);
Node *new_Integer(char *value, int len, int line);
Node *new_Boolean(char *value, int len, int line);
Node *new_String(char *value, int len, int line);
Node *List_get_values(Node *node);
Node *new_List(Node *values, int line);
Node *Table_get_keys(Node *node);
Node *Table_get_values(Node *node);
Node *new_Table(Node *keys, Node *values, int line);

void node_del(Node *node);
