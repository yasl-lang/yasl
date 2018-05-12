#pragma once

#include "../token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NODE_EXPRSTMT,
    NODE_BLOCK,
    NODE_WHILE,
    NODE_BREAK,
    NODE_CONT,
    NODE_IF,
    NODE_PRINT,
    NODE_LET,
    NODE_TRIOP,
    NODE_BINOP,
    NODE_UNOP,
    NODE_ASSIGN,
    NODE_VAR,
    NODE_UNDEF,
    NODE_FLOAT64,
    NODE_INT64,
    NODE_BOOL,
    NODE_STR,
    NODE_LIST,
    NODE_MAP
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

Node *new_Node_0(AST nodetype, Token type, char *name, int64_t name_len, int line);
Node *new_Node_1(AST nodetype, Token type, Node *child, char *name, int64_t name_len, int line);
Node *new_Node_2(AST nodetype, Token type, Node *child1, Node *child2, char *name, int64_t name_len, int line);
Node *new_Node_3(AST nodetype, Token type, Node *child1, Node *child2, Node *child3, char *name, int64_t name_len, int line);

Node *new_ExprStmt(Node *child, int line);
Node *new_Block(int line);
Node *new_While(Node *cond, Node *body);
Node *new_Break(void);
Node *new_Continue(void);
Node *new_If(Node *cond, Node *then_node, Node *else_node);
Node *new_Print(Node *child);
Node *new_Let(char *name, int64_t name_len, Node *child);
Node *new_TriOp(Token op, Node *left, Node *middle, Node *right);
Node *new_BinOp(Token op, Node *left, Node *right);
Node *new_UnOp(Token op, Node *child);
Node *new_Assign(char *name, int64_t name_len, Node *child);
Node *new_Var(char *name, int64_t name_len);
Node *new_Undef(void);
Node *new_Float(char *value, int len);
Node *new_Integer(char *value, int len);
Node *new_Boolean(char *value, int len);
Node *new_String(char *value, int len);
Node *new_List(Node *values);
Node *new_Map(Node *keys, Node *values);

void del_Block(Node *node);
void del_While(Node *node);
void del_Break(Node *node);
void del_Continue(Node *node);
void del_If(Node *node);
void del_Print(Node *node);
void del_Let(Node *node);
void del_TriOp(Node *node);
void del_BinOp(Node *node);
void del_Undef(Node *node);
void del_Float(Node *node);
void del_Integer(Node *node);
void del_String(Node *node);
void del_List(Node *node);
void del_Map(Node *node);
void node_del(Node *node);


/*
 * class AST(object):
    pass
class Hash(AST):
    def __init__(self, token, keys, vals):
        self.token = token
        self.value = token.value
        self.keys = keys
        self.vals = vals
class List(AST):
    def __init__(self, token, params):
        self.token = token
        self.value = token.value
        self.params = params
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


/*


typedef struct {
    AST nodetype;
    int type;
    void *left;
    void *middle;
    void *right;
} TriOpNode;

TriOpNode *ast_new_TriOpNode(int type, void *left, void *middle, void *right) {
    TriOpNode *triop = malloc(sizeof(TriOpNode));
    triop->nodetype = TRIOP;
    triop->type = type;
    triop->left = left;
    triop->right = right;
    triop->middle = middle;
    return triop;
}

typedef struct {
    AST nodetype;
    int type;
    void *left;
    void *right;
} BinOpNode;

typedef struct {
    AST nodetype;
    char *value;
} ConstantNode;

 */
