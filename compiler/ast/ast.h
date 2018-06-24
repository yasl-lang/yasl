#pragma once

#include "../token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    N_EXPRSTMT,
    N_BLOCK,
    N_FNDECL,
    N_RET,
    N_CALL,
    N_METHOD,
    N_INDEX,
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

void block_append(Node *node, Node *child);

Node *clone_node(Node *node);
Node *new_Node_0(AST nodetype, Token type, char *name, int64_t name_len, int line);
Node *new_Node_1(AST nodetype, Token type, Node *child, char *name, int64_t name_len, int line);
Node *new_Node_2(AST nodetype, Token type, Node *child1, Node *child2, char *name, int64_t name_len, int line);
Node *new_Node_3(AST nodetype, Token type, Node *child1, Node *child2, Node *child3, char *name, int64_t name_len, int line);

Node *new_ExprStmt(Node *child, int line);
Node *new_Block(int line);
Node *new_FunctionDecl(Node *params, Node *body, char *name, int64_t name_len, int line);
Node *new_Return(Node *expr, int line);
Node *new_FunctionCall(Node *params, Node *object, int line);
Node *new_MethodCall(Node *object, Node *params, char *name, int64_t name_len, int line);
Node *new_Index(Node *collection, Node *value, int line);
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


/*
 * class AST(object):
    pass
class MemberAccess(AST):
    def __init__(self, left, right):
        self.left = left
        self.right = right
class MethodCall(AST):
    def __init__(self, left, right, params):
        self.left = left
        self.right = right
        self.value = right.value
        self.params = params
class Index(AST):
    def __init__(self, left, right):
        self.left = left
        self.right = right
        self.value = left.value
class FunctionDecl(AST):
    def __init__(self, token, params, block):
        self.token = token
        self.value = token.value
        self.params = params # list of formal params
        self.block = block
class FunctionCall(AST):
    def __init__(self, token, params):
        self.token = token
        self.value = token.value
        self.params = params
class Return(AST):
    def __init__(self, token, expr):
        self.token = token
        self.expr = expr
class For(AST):
    def __init__(self, token, var, ls, body):
        self.token = token
        self.var = var
        self.ls = ls
        self.body = body
 */