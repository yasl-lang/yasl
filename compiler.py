from opcode import *
from tokens import *
from ast import *
from parser import Parser
from lexer import Lexer
import pickle
import struct
import os
from visitor import NodeVisitor
from environment import Env

BINRESERVED = {
        "+":  [ADD],
        "*":  [MUL],
        "-":  [SUB],
        "/":  [DIV],
        "<":  [GE, NOT],
        "<=": [GT, NOT],
        ">":  [GT],
        ">=": [GE],
        "==": [EQ],
        "!=": [EQ, NOT],
        }
UNRESERVED = {
        "-": [NEG],
        "!": [NOT],
        }
ICONSTANTS = {
        #-2: [ICONST_M2],
        -1: [ICONST_M1],
        0: [ICONST_0],
        1: [ICONST_1],
        2: [ICONST_2],
        3: [ICONST_3],
        4: [ICONST_4],
        5: [ICONST_5],
        #6: [ICONST_6],
        }
DCONSTANTS = {
        0.0: [DCONST_0],
        1.0: [DCONST_1],
        2.0: [DCONST_2],
        }

def intbytes_8(n:int):
    return [int(b) for b in bytearray(struct.pack("@q", n))]
def doublebytes(d:float):
    return [int(b) for b in bytearray(struct.pack("@d", d))]
###############################################################################
#                                                                             #
#  INTERPRETER                                                                #
#                                                                             #
###############################################################################

class Compiler(NodeVisitor):
    def __init__(self):
        self.env = self.globals = Env()
        self.header = intbytes_8(8)
        self.fns = {}
        self.fn_lens = {}
        self.fn_locals = {}
        self.locals = Env(self.globals)
        self.offset = 0
    def compile(self, statements):
        #print(ICONSTANTS)
        result = []
        for statement in statements:
            #print(ICONSTANTS)
            #print(statement)
            result = result + self.visit(statement)
        self.header[0:8] = intbytes_8(len(self.header))
        for opcode in self.header: print(hex(opcode))
        print("entry point:")
        for opcode in result + [HALT]: print(hex(opcode))
        return self.header + result + [HALT] #TODO: fix return values once we have proper ones
    def visit_Print(self, node):
        expr = self.visit(node.expr)
        return expr + [PRINT]
    def visit_Block(self, node):
        result = []
        for statement in node.statements:
            result = result + self.visit(statement)
        return result
    def visit_ExprStmt(self, node):
        expr = self.visit(node.expr)
        return expr + [POP]
    def visit_If(self, node):
        cond = self.visit(node.cond)
        self.env = Env(self.env)
        body = self.visit(node.body)
        self.env = self.env.parent
        return cond + [BRF_8] + intbytes_8(len(body)) + body
    def visit_IfElse(self, node):
        cond = self.visit(node.cond)
        self.env = Env(self.env)
        left = self.visit(node.left)
        self.env = self.env.parent
        self.env = Env(self.env)
        right = self.visit(node.right)
        self.env = self.env.parent
        left = left + [BR_8] + intbytes_8(len(right))
        return cond + [BRF_8] + intbytes_8(len(left)) + left + right
    def visit_While(self, node):
        cond = self.visit(node.cond)
        self.env = Env(self.env)
        body = self.visit(node.body)
        self.env = self.env.parent
        cond = cond + [BRF_8] + intbytes_8(len(body)+9)
        body = body + [BR_8] + intbytes_8(-(len(body)+9+len(cond)))
        return cond + body
    def visit_TriOp(self, node): #only 1 tri-op is possible
        cond = self.visit(node.cond)
        left = self.visit(node.left)
        right = self.visit(node.right)
        left = left + [BR_8] + intbytes_8(len(right))
        return cond + [BRF_8] + intbytes_8(len(left)) + left + right
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if node.token.value == "??":
            left = left + [DUP]
            right = [POP] + right
            return left + [ISNIL, BRF_8] + intbytes_8(len(right)) + right
        this = BINRESERVED.get(node.op.value)
        return left + right + this
    def visit_LogicOp(self, node):
        left = self.visit(node.left)
        right = [POP] + self.visit(node.right)
        left = left + [DUP]
        if node.op.value == "and":
            return left + [BRF_8] + intbytes_8(len(right)) + right
        elif node.op.value == "or":
            return left + [BRT_8] + intbytes_8(len(right)) + right
        else:
            assert False
    def visit_UnOp(self, node):
        expr = self.visit(node.expr)
        this = UNRESERVED.get(node.op.value)
        return expr + this
    '''def visit_NulOp(self, node):
        pass
    def visit_FuncDecl(self, node):
        pass
    def visit_FuncCall(self, node):
        pass '''
    def visit_FunctionDecl(self, node):
        if self.env is not self.globals:
            raise Exception("cannot declare function outside of global scope. (line %s)" % node.token.line)
        if node.token.value in self.fns:
            raise Exception("invalid redefinition of function %s (line %s)" % (node.token.value, node.token.line))
        self.fns[node.token.value] = len(self.header)
        self.fn_lens[node.token.value] = node.params.__len__()
        self.env = self.locals
        self.offset = self.fn_lens[node.token.value]
        for i in node.params:
            self.env[i.value] = 255 - len(self.env.vars) - 1
        self.fn_locals[node.token.value] = len(self.env.vars)
        for stmt in node.block.statements:
            self.header = self.header + self.visit(stmt)
        self.header = self.header + [NCONST, RET]
        self.env = self.globals
        self.locals = Env(self.globals)
        return []
    def visit_FunctionCall(self, node):
        result = []
        for expr in node.params:
            result = result + self.visit(expr)
        return result + [CALL_8, self.fn_lens[node.token.value]] + \
                         intbytes_8(self.fns[node.token.value]) + \
                         [self.fn_locals[node.token.value]]
    def visit_Return(self, node):
        expr = self.visit(node.expr)
        return expr + [RET]
    def visit_Decl(self, node):
        #print(self.env is self.locals)
        if self.env is self.locals:
            if node.left.value not in self.env.vars:
                self.env[node.left.value] = len(self.env.vars) + 1 - self.offset
            return self.visit(node.right) + [LSTORE_1, self.env[node.left.value]]
        if node.left.value not in self.env.vars:
            self.env.decl_var(node.left.value)
        #print(node.right)
        right = self.visit(node.right)
        #print([hex(r) for r in right])
        result = self.visit(node.right) + [GSTORE_1, self.env[node.left.value]]
        #print([hex(r) for r in result])
        return result
        return self.visit(node.right) + [GSTORE_1, self.env[node.left.value]]
    def visit_Assign(self, node):
        if node.left.value not in self.env:
            raise Exception("undeclared variable: %s in line %s" % (node.left.value, node.left.line))
        #print(self.env is self.locals)
        if self.env is self.locals:
            return self.visit(node.right) + [LSTORE_1,
                                             self.env[node.left.value],
                                             LLOAD_1,
                                             self.env[node.left.value]]
        return self.visit(node.right) + [GSTORE_1, self.env[node.left.value],
                                         GLOAD_1, self.env[node.left.value]]
    def visit_Var(self, node):
        if node.value not in self.env:
            raise Exception("undefined variable: %s in line %s" % (node.value, node.token.line))
        if self.env is self.locals:
            return [LLOAD_1, self.env[node.value]]
        return [GLOAD_1, self.env[node.value]]
    '''def visit_String(self, node):
        pass '''
    def visit_Boolean(self, node):
        if node.value == False:
            return [BCONST_F]
        elif node.value == True:
            return [BCONST_T]
        else:
            raise Exception("invalid boolean")
    def visit_Integer(self, node):
        if node.value in ICONSTANTS:
            return ICONSTANTS[node.value]
        return [ICONST] + intbytes_8(node.value)
    def visit_Float(self, node):
        if node.value in DCONSTANTS:
            return DCONSTANTS[node.value]
        return [DCONST] + doublebytes(node.value)
    def visit_Nil(self, node):
        return [NCONST]
