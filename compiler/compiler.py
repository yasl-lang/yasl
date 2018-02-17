from .opcode import *
from .constant import *
import struct
from .visitor import NodeVisitor
from .environment import Env

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
        "===": [ID],
        "!==": [ID, NOT]
        }
UNRESERVED = {
        "-": [NEG],
        "!": [NOT],
        "+": [NOP],
        "#": [LEN],
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
BUILTINS = {
        "upcase": 0,
        "downcase": 1,
        "isalnum": 2,
        "isal": 3,
        "isnum": 4,
        "isspace": 5,
        "insert": 6,
        "find": 7,
        "append": 8,
}

def intbytes_8(n:int):
    return [int(b) for b in bytearray(struct.pack("@q", n))]
def doublebytes(d:float):
    return [int(b) for b in bytearray(struct.pack("@d", d))]

###############################################################################
#                                                                             #
#  COMPILER                                                                   #
#                                                                             #
###############################################################################

class Compiler(NodeVisitor):
    def __init__(self):
        self.env = self.globals = Env()
        self.header = intbytes_8(8) + intbytes_8(0)
        self.fns = {}
        self.current_fn = None
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
        self.header[8:16] = intbytes_8(len(self.globals))  # TODO: fix so this works with locals as well
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
        print(left, right)
        if node.token.value == "??":
            left = left + [DUP]
            right = [POP] + right
            return left + [ISNIL, BRF_8] + intbytes_8(len(right)) + right
        elif node.token.value == "||":
            return left + [V2S, DUP, LEN, ICONST_0, SWAP] + right + [V2S, DUP, LEN, SWAP_X1, DUP2, ADD, DUP, ICONST] + \
                   intbytes_8(8) + [ADD, MLC, STR, ICP, SCP, SCP]
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
        self.current_fn = node.token.value
        if self.env is not self.globals:
            raise Exception("cannot declare function outside of global scope. (line %s)" % node.token.line)
        if node.token.value in self.fns:
            raise Exception("invalid redefinition of function %s (line %s)" % (node.token.value, node.token.line))
        self.fns[node.token.value] = {}
        self.fns[node.token.value]["addr"] = len(self.header)
        self.fns[node.token.value]["params"] = node.params.__len__()
        self.env = self.locals
        self.offset = self.fns[node.token.value]["params"]
        for i in node.params:
            self.env[i.value] = 255 - len(self.env.vars) - 1
        self.fns[node.token.value]["locals"] = len(self.env.vars)
        for stmt in node.block.statements:
            self.header = self.header + self.visit(stmt)
        self.header = self.header + [NCONST, RET]
        self.env = self.globals
        self.locals = Env(self.globals)
        self.current_fn = None  # TODO: allow nested function definitions
        return []
    def visit_FunctionCall(self, node):
        result = []
        for expr in node.params:
            result = result + self.visit(expr)
        # print([hex(r) for r in result])
        if node.value in BUILTINS:
            return result + [BCALL_8] + intbytes_8(BUILTINS[node.value])
        return result + [CALL_8, self.fns[node.token.value]["params"]] + \
                         intbytes_8(self.fns[node.token.value]["addr"]) + \
                         [self.fns[node.token.value]["locals"]]
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
    def visit_Assign(self, node):
        if node.left.value not in self.env:
            raise Exception("undeclared variable: %s in line %s" % (node.left.value, node.left.line))
        #print(self.env is self.locals)
        if self.env is not None:
            return self.visit(node.right) + [LSTORE_1,
                                             self.env[node.left.value],
                                             LLOAD_1,
                                             self.env[node.left.value]]
        return self.visit(node.right) + [GSTORE_1, self.env[node.left.value],
                                         GLOAD_1, self.env[node.left.value]]
    def visit_Var(self, node):
        if node.value not in self.env:
            raise Exception("undefined variable: %s in line %s" % (node.value, node.token.line))
        if self.current_fn is not None:
            return [LLOAD_1, self.env[node.value]]
        return [GLOAD_1, self.env[node.value]]
    def visit_Hash(self, node):
        return [NEWHASH]  # TODO: allow declaration with a bunch of values in it
    def visit_List(self, node):
        return [NEWLIST]
    def visit_String(self, node):
        string = [int(b) for b in str.encode(node.value)]
        length = intbytes_8(len(string))
        length8 = intbytes_8(len(string)+8)
        return [MLC_8, STR] + length8 + [MCP_8] + intbytes_8(0) + length8 + length + string
        '''MLC,
        0x30,
        0x14,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        MCP_8,
        0x00,
        0x14,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x48,
        0x45,
        0x4C,
        0x4C,
        0x4F,
        0x20,
        0x57,
        0x4F,
        0x52,
        0x4C,
        0x44,
        0x2E,'''
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
    def visit_Undef(self, node):
        return [NCONST]
