#include <stdio.h>
#include <stdlib.h>


struct Node_s {
    AST nodetype;
    Tokens type;
    Node_s *children;
    char* name;
    int len;
};

typedef struct Node_s Node;

Node *new_TriOp(Token op, left, middle, right) {

}
/*
 * class AST(object):
    pass
class TriOp(AST):
    def __init__(self, op, cond, left, right):
        self.left = left
        self.cond = cond
        self.right = right
        self.token = self.op = op
class BinOp(AST):
    def __init__(self, op, left, right):
        self.left = left
        self.right = right
        self.token = self.op = op
class LogicOp(AST):
    def __init__(self, op, left, right):
        self.left = left
        self.right = right
        self.token = self.op = op
class UnOp(AST):
    def __init__(self, op, expr):
        self.token = self.op = op
        self.expr = expr
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
class String(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
class Undef(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
class Boolean(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
class Integer(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
class Float(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
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
'''
class FuncCall(AST):
    def __init__(self, token, params):
        self.token = token
        self.value = token.value
        self.params = params'''
class Decl(AST):
    def __init__(self, left, right):
        self.left = left
        self.right = right
class Assign(AST):
    def __init__(self, left, right):
        self.left = left  # ID
        self.right = right # val
class Var(AST):
    # ID Token is used here
    def __init__(self, token):
        self.token = token
        self.value = token.value
class Print(AST):
    def __init__(self, token, expr):
        self.token = token
        self.expr = expr
class Input(AST):
    def __init__(self, token, expr):
        self.token = token
        self.expr = expr
class If(AST):
    def __init__(self, token, cond, body):
        self.token = token
        self.cond = cond
        self.body = body
class IfElse(AST):
    def __init__(self, token, cond, left, right):
        self.token = token
        self.cond = cond
        self.left = left
        self.right = right
class While(AST):
    def __init__(self, token, cond, body):
        self.token = token
        self.cond = cond
        self.body = body
class For(AST):
    def __init__(self, token, var, ls, body):
        self.token = token
        self.var = var
        self.ls = ls
        self.body = body
class Break(AST):
    def __init__(self, token):
        self.token = token
class Continue(AST):
    def __init__(self, token):
        self.token = token
class Block(AST):
    def __init__(self, statements):
        self.statements = statements  # list of ASTs to iterate over
class ExprStmt(AST):
    def __init__(self, expr):
        self.expr = expr
 */




/* typedef enum {
    TRIOP,
    BINOP,
    UNOP,
} AST;




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
