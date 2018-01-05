from opcode import *
from tokens import *
from ast import *
from parser import Parser
from lexer import Lexer
import pickle
import struct
import os
from visitor import NodeVisitor
from collections import OrderedDict

BINRESERVED = {
        "+": ADD,
        "*": MUL,
        "-": SUB,
        "/": DIV,
        }
UNRESERVED = {
        "-": NEG,
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

###############################################################################
#                                                                             #
#  INTERPRETER                                                                #
#                                                                             #
###############################################################################

class Interpreter(NodeVisitor):
    def __init__(self):
        self.globals = OrderedDict()
    def interpret(self, statements):
        result = []
        for statement in statements:
            result += self.visit(statement)
        return result #TODO: fix return values once we have proper ones
    '''def visit_TriOp(self, node):
        pass'''
    def visit_Print(self, node):
        expr = self.visit(node.expr)
        return expr + [PRINT]
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        this = BINRESERVED.get(node.op.value)
        return left + right + [this]
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
        if node.left.value not in self.globals:
            self.globals[node.left.value] = len(self.globals)
        return self.visit(node.right) + [GSTORE, self.globals.get(node.left.value)]
    '''def visit_Assign(self, node):
        if node.left.value not in self.globals:
            print("undeclared variable: %s" % node.left.value)
            assert False
        return self.visit(node.right) + [GSTORE, self.globals.get(node.left.value)]'''
    def visit_Var(self, node):
        if node.value not in self.globals:
            assert False
        return [GLOAD, self.globals.get(node.value)]
    '''def visit_String(self, node):
        pass '''
    def visit_Boolean(self, node):
        if node.value == False:
            return [FALSE]
        elif node.value == True:
            return [TRUE]
        else:
            assert False
    def visit_Integer(self, node):
        if node.value in ICONSTANTS:
            return [ICONSTANTS[node.value]]
        return [ICONST] + [ int(b) for b in bytearray(struct.pack(">q", node.value)) ]
    def visit_Float(self, node):
        if node.value in DCONSTANTS:
            return [DCONSTANTS[node.value]]
        return [DCONST] + [ int(b) for b in bytearray(struct.pack(">d", node.value)) ]
