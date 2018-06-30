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
    N_MAP
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

Node *new_ExprStmt(Node *child, int line);
Node *new_Block(Node *body, int line);
Node *new_Body(int line);
Node *new_FunctionDecl(Node *params, Node *body, char *name, int64_t name_len, int line);
Node *new_Return(Node *expr, int line);
Node *new_Set(Node *collection, Node *key, Node *value, int line);
Node *new_Get(Node *collection, Node *value, int line);
Node *new_FunctionCall(Node *params, Node *object, int line);
Node *new_While(Node *cond, Node *body, int line);
Node *new_Break(int line);
Node *new_Continue(int line);
Node *new_If(Node *cond, Node *then_node, Node *else_node, int line);
Node *new_Print(Node *child, int line);
Node *new_Let(char *name, int64_t name_len, Node *child, int line);
Node *new_TriOp(Token op, Node *left, Node *middle, Node *right, int line);
Node *new_BinOp(Token op, Node *left, Node *right, int line);
Node *new_UnOp(Token op, Node *child, int line);
Node *new_Assign(char *name, int64_t name_len, Node *child, int line);
Node *new_Var(char *name, int64_t name_len, int line);
Node *new_Undef(int line);
Node *new_Float(char *value, int len, int line);
Node *new_Integer(char *value, int len, int line);
Node *new_Boolean(char *value, int len, int line);
Node *new_String(char *value, int len, int line);
Node *new_List(Node *values, int line);
Node *new_Map(Node *keys, Node *values, int line);

void node_del(Node *node);
