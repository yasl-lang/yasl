from tokens import *

class AST(object):
    def ret_type(self):
        return None
'''class TriOp(AST):
    def __init__(self, op, left, middle, right):
        self.left = left
        self.middle = middle
        self.right = right
        self.token = self.op = op'''
class BinOp(AST):
    def __init__(self, op, left, right):
        self.left = left
        self.right = right
        self.token = self.op = op
    def ret_type(self):
        if self.op.value == "/":
            return FLOAT
        if self.op.value in ["+", "-", "*"]:
            if self.left.ret_type() == FLOAT or self.right.ret_type() == FLOAT:
                return FLOAT
            return INT
class UnOp(AST):
    def __init__(self, op, expr):
        self.token = self.op = op
        self.expr = expr
    def ret_type(self):
        return self.expr.ret_type()
'''class NulOp(AST):
    def __init__(self, op):
        self.token = self.op = op
class String(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
class Boolean(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value'''
class Integer(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
    def ret_type(self):
        return INT
class Float(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value
    def ret_type(self):
        return FLOAT
'''class Param(AST):
    def __init__(self, var_node):
        self.node = var_node
class FuncDecl(AST):
    def __init__(self, token, params, block):
        self.token = token
        self.value = token.value
        self.params = params # list of formal params
        self.block = block
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
    def ret_type(self):
        return self.right.ret_type()
class Var(AST):
    # ID Token is used here
    def __init__(self, token):
        self.token = token
        self.value = token.value
    def ret_type(self):
        return INT #TODO: replace with proper value
class Print(AST):
    def __init__(self, token, expr):
        self.token = token
        self.expr = expr
