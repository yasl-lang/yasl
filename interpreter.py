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
        "+": ADD,
        "*": MUL,
        "-": SUB,
        "/": DIV,
        "<": LT,
        "<=": LE,
        ">": GT,
        ">=": GE,
        "==": EQ,
        "!=": NEQ,
        }
UNRESERVED = {
        "-": NEG,
        "!": NOT,
        }
ICONSTANTS = {
        -1: ICONST_M1,
        0: ICONST_0,
        1: ICONST_1,
        2: ICONST_2,
        3: ICONST_3,
        4: ICONST_4,
        5: ICONST_5,
        }
DCONSTANTS = {
        0.0: DCONST_0,
        1.0: DCONST_1,
        2.0: DCONST_2,
        }

def intbytes_8(n:int):
    return [int(b) for b in bytearray(struct.pack(">q", n))]
def doublebytes(d:float):
    return [int(b) for b in bytearray(struct.pack(">d", n))]
###############################################################################
#                                                                             #
#  INTERPRETER                                                                #
#                                                                             #
###############################################################################

class Interpreter(NodeVisitor):
    def __init__(self):
        self.env = self.globals = Env()
    def interpret(self, statements):
        result = []
        for statement in statements:
            result += self.visit(statement)
        for opcode in result: print(hex(opcode))
        return result #TODO: fix return values once we have proper ones
    def visit_Print(self, node):
        expr = self.visit(node.expr)
        return expr + [PRINT]
    def visit_Block(self, node):
        result = []
        for statement in node.statements:
            result += self.visit(statement)
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
        left += [BR_8] + intbytes_8(len(right))
        return cond + [BRF_8] + intbytes_8(len(left)) + left + right
    def visit_While(self, node):
        cond = self.visit(node.cond)
        self.env = Env(self.env)
        body = self.visit(node.body)
        self.env = self.env.parent
        cond += [BRF_8] + intbytes_8(len(body)+9)
        body += [BR_8] + intbytes_8(-(len(body)+9+len(cond)))
        return cond + body
    def visit_TriOp(self, node): #only 1 tri-op is possible
        cond = self.visit(node.cond)
        left = self.visit(node.left)
        right = self.visit(node.right)
        left += [BR_8] + intbytes_8(len(right))
        return cond + [BRF_8] + intbytes_8(len(left)) + left + right
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        this = BINRESERVED.get(node.op.value)
        return left + right + [this]
    def visit_LogicOp(self, node):
        left = self.visit(node.left)
        right = [POP] + self.visit(node.right)
        left += [DUP]
        if node.op.value == "and":
            return left + [BRF_8] + intbytes_8(len(right)) + right
        elif node.op.value == "or":
            return left + [BRT_8] + intbytes_8(len(right)) + right
        else:
            assert False
    def visit_UnOp(self, node):
        expr = self.visit(node.expr)
        this = UNRESERVED.get(node.op.value)
        return expr + [this]
    '''def visit_NulOp(self, node):
        pass
    def visit_FuncDecl(self, node):
        pass
    def visit_FuncCall(self, node):
        pass '''
    def visit_Decl(self, node):
        if node.left.value not in self.env.vars:
            self.env.decl_var(node.left.value)
        return self.visit(node.right) + [GSTORE, self.env[node.left.value]]
    def visit_Assign(self, node):
        if node.left.value not in self.env:
            raise Exception("undeclared variable: %s" % node.left.value)
        return self.visit(node.right) + [GSTORE, self.env[node.left.value], GLOAD, self.env[node.left.value]]
    def visit_Var(self, node):
        if node.value not in self.env:
            raise Exception("undefined variable: %s" % node.value)
        return [GLOAD, self.env[node.value]]
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
            return [ICONSTANTS[node.value]]
        return [ICONST] + intbytes_8(node.value)
    def visit_Float(self, node):
        if node.value in DCONSTANTS:
            return [DCONSTANTS[node.value]]
        return [DCONST] + doublebytes(node.value)

    # noinspection PyInterpreter
    def visit_Nil(self, node):
        return [NCONST]
