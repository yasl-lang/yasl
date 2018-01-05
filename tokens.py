from enum import Enum #TODO: change to enum

class TokenTypes(Enum):
    INT    = 0x00
    FLOAT  = 0x01
    BOOL   = 0x02
    VAR    = 0x10
    OP     = 0x18
    ID     = 0x20
    LPAREN = 0x30
    RPAREN = 0x31
    QMARK  = 0x36
    COLON  = 0x37
    DOT    = 0x38
    COMMA  = 0x39
    ARROW  = 0x3A
    SEMI   = 0x3B
    EOF    = 0x3F
    PRINT  = 0x50


'''
ID, LPAREN, RPAREN, SEMI, EOF = \
"ID", "LPAREN", "RPAREN", "SEMI", "EOF"
DEFUN, COLON, ARROW, COMMA = "DEFUN", "COLON", "ARROW", "COMMA"
QMARK = "QMARK"
PRINT = "PRINT"
OP = "OP"
INT, FLOAT, STR, BOOL = "INT", "FLOAT", "STR", "BOOL"
VAR = "VAR" '''


class Token(object):
    def __init__(self, token_type, token_value):
        # token types listed at top of file
        self.type = token_type
        # token values: e.g. MATH412
        self.value = token_value
    def __str__(self):
        return "Token(%s, %s)" % (self.type, repr(self.value))
    __repr__ = __str__
