from .opcode import *
from .constant import *
import struct
from .visitor import NodeVisitor
from .environment import Env
from .ast import *

BINRESERVED = {
        "+":   [ADD],
        "*":   [MUL],
        "-":   [SUB],
        "/":   [DIV],
        "%":   [MOD],
        "||":  [CONCAT],
        "<":   [GE, NOT],
        "<=":  [GT, NOT],
        ">":   [GT],
        ">=":  [GE],
        "==":  [EQ],
        "!=":  [EQ, NOT],
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
        -1: [ICONST_M1],
        0:  [ICONST_0],
        1:  [ICONST_1],
        2:  [ICONST_2],
        3:  [ICONST_3],
        4:  [ICONST_4],
        5:  [ICONST_5],
        }
DCONSTANTS = {
        0.0: [DCONST_0],
        1.0: [DCONST_1],
        2.0: [DCONST_2],
        }
STDIO = {
    "print": 0x00,
    #"input": 0x01,
    # "open": 0x02,
    # "close": 0x03,

}
STDSTR = {
    "upcase":   0x00,
    "downcase": 0x01,
    "isalnum":  0x02,
    "isal":     0x03,
    "isnum":    0x04,
    "isspace":  0x05,
    "split":    0x06,
}
STDOBJ = {
    "insert": 0x00,
    "find":   0x01,
    "append": 0x02,

}
BUILTINS = {
        "print":      0x00,
        "upcase":     0x01,
        "downcase":   0x02,
        "isalnum":    0x03,
        "isal":       0x04,
        "isnum":      0x05,
        "isspace":    0x06,
        "startswith": 0x07,
        "endswith":   0x08,
        "search":     0x09,
        "insert":     0x0A,
        "find":       0x0B,
        "keys":       0x0C,
        "append":     0x0D,
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
        self.globals = Env()
        self.locals = Env()
        self.header = intbytes_8(8) + intbytes_8(0)
        self.fns = {}
        self.current_fn = None
        self.offset = 0
    def compile(self, statements):
        result = []
        for statement in statements:
            result = result + self.visit(statement)
        self.header[0:8] = intbytes_8(len(self.header))
        self.header[8:16] = intbytes_8(len(self.globals))  # TODO: fix so this works with locals as well
        for opcode in self.header: print(hex(opcode))
        print("entry point:")
        for opcode in result + [HALT]: print(hex(opcode))
        return self.header + result + [HALT] #TODO: fix return values once we have proper ones
    def enter_scope(self):
        if self.current_fn is None:
            self.locals = Env(self.locals)
        else:
            self.globals = Env(self.globals)
    def exit_scope(self):
        if self.current_fn is None:
            self.locals = self.locals.parent
        else:
            self.globals = self.globals.parent
    def visit_Print(self, node):
        expr = self.visit(node.expr)
        return expr + [BCALL_8] + intbytes_8(BUILTINS["print"])
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
        self.enter_scope()
        body = self.visit(node.body)
        self.exit_scope()
        return cond + [BRF_8] + intbytes_8(len(body)) + body
    def visit_IfElse(self, node):
        cond = self.visit(node.cond)
        self.enter_scope()
        left = self.visit(node.left)
        self.exit_scope()
        self.enter_scope()
        right = self.visit(node.right)
        self.exit_scope()
        left = left + [BR_8] + intbytes_8(len(right))
        return cond + [BRF_8] + intbytes_8(len(left)) + left + right
    def visit_While(self, node):
        cond = self.visit(node.cond)
        self.enter_scope()
        body = self.visit(node.body)
        self.exit_scope()
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
            return left + [DUP, BRN_8] + intbytes_8(len(right)+1) + [POP] + right
        '''elif node.token.value == "||":
            return left + [V2S, DUP, LEN, ICONST_0, SWAP] + right + [V2S, DUP, LEN, SWAP_X1, DUP2, ADD, DUP, ICONST] + \
                   intbytes_8(8) + [ADD, MLC, STR8, ICP, SCP, SCP]'''
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
    def visit_FunctionDecl(self, node):
        if self.current_fn is not None:
            raise Exception("cannot declare function outside of global scope. (line %s)" % node.token.line)
        self.current_fn = node.token.value
        if node.token.value in self.fns:
            raise Exception("invalid redefinition of function %s (line %s)" % (node.token.value, node.token.line))
        self.fns[node.token.value] = {}
        self.fns[node.token.value]["addr"] = len(self.header)
        self.fns[node.token.value]["params"] = node.params.__len__()
        self.locals = Env()
        self.offset = self.fns[node.token.value]["params"]
        node.params.reverse()
        for i in node.params:
            self.locals[i.value] = 255 - len(self.locals.vars) - 1
        self.fns[node.token.value]["locals"] = len(self.locals.vars)
        for stmt in node.block.statements:
            self.header = self.header + self.visit(stmt)
        self.header = self.header + [NCONST, RET]
        self.locals = Env()
        self.current_fn = None  # TODO: allow nested function definitions
        return []
    def visit_FunctionCall(self, node):
        result = []
        for expr in node.params:
            result = result + self.visit(expr)
        if node.value in BUILTINS:
            return result + [BCALL_8] + intbytes_8(BUILTINS[node.value])
        return result + [CALL_8, self.fns[node.token.value]["params"]] + \
                         intbytes_8(self.fns[node.token.value]["addr"]) + \
                         [self.fns[node.token.value]["locals"]]
    def visit_Return(self, node):
        if isinstance(node.expr, FunctionCall) and node.expr.value == self.current_fn:
            result = []
            for param in node.expr.params:
                result = result + self.visit(param)
            return result + [RCALL_8, self.fns[node.expr.value]["params"]] + \
                        intbytes_8(self.fns[node.expr.value]["addr"]) + \
                        [self.fns[node.expr.value]["locals"]]
        return self.visit(node.expr) + [RET]
    def visit_Decl(self, node):
        if self.current_fn is not None:
            if node.left.value not in self.locals.vars:
                self.locals[node.left.value] = len(self.locals.vars) + 1 - self.offset
            return self.visit(node.right) + [LSTORE_1, self.locals[node.left.value]]
        if node.left.value not in self.globals.vars:
            self.globals.decl_var(node.left.value)
        right = self.visit(node.right)
        result = right + [GSTORE_1, self.globals[node.left.value]]
        return result
    def visit_Assign(self, node):
        if node.left.value not in self.locals and node.left.value not in self.globals:
            raise Exception("undeclared variable: %s in line %s" % (node.left.value, node.left.token.line))
        if isinstance(node.left, Var):
            if self.current_fn is not None:
                if node.left.value in self.locals:
                    return self.visit(node.right) + [LSTORE_1,
                           self.locals[node.left.value],
                           LLOAD_1,
                           self.locals[node.left.value]]
            return self.visit(node.right) + [GSTORE_1, self.globals[node.left.value],
                                             GLOAD_1, self.globals[node.left.value]]
        elif isinstance(node.left, Index):
            index = node.left
            return self.visit(index.left) + self.visit(index.right) + self.visit(node.right) + \
                   [BCALL_8] + intbytes_8(BUILTINS["insert"])
    def visit_Var(self, node):
        if node.value not in self.locals and node.value not in self.globals:
            raise Exception("undefined variable: %s in line %s" % (node.value, node.token.line))
        if (self.current_fn is not None) and (node.value in self.locals):
            return [LLOAD_1, self.locals[node.value]]
        return [GLOAD_1, self.globals[node.value]]
    def visit_Index(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        return left + right + [BCALL_8] + intbytes_8(BUILTINS["find"])
    def visit_Hash(self, node):
        result = [NEWHASH]
        for i in range(len(node.keys)):
            result = result + [DUP] + self.visit(node.keys[i]) + self.visit(node.vals[i]) + [BCALL_8] + intbytes_8(BUILTINS["insert"]) + [POP]
        return result  # TODO: allow declaration with a bunch of values in it
    def visit_List(self, node):
        result = [NEWLIST]
        for expr in node.params:
            result = result + [DUP] + self.visit(expr) + [BCALL_8] + intbytes_8(BUILTINS["append"]) + [POP]
        return result
    def visit_String(self, node):
        string = [int(b) for b in str.encode(node.value)]
        length = intbytes_8(len(string))
        return [NEWSTR8] + length + string
        #length8 = intbytes_8(len(string)+8)
        #return [MLC_8, STR8] + length8 + [MCP_8] + intbytes_8(0) + length8 + length + string
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
