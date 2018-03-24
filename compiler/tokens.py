from enum import Enum

class TokenTypes(Enum):
    INT    = 0x00
    FLOAT  = 0x01
    BOOL   = 0x02
    UNDEF  = 0x03
    STR    = 0x04
    MAP    = 0x05
    LIST   = 0x06
    IF     = 0x10
    ELSEIF = 0x11
    ELSE   = 0x12
    WHILE  = 0x13
    FOR    = 0x14
    OP     = 0x18
    LOGIC  = 0x19
    ID     = 0x20
    LPAREN = 0x30
    RPAREN = 0x31
    LBRACK = 0x32
    RBRACK = 0x33
    LBRACE = 0x34
    RBRACE = 0x35
    QMARK  = 0x36
    COLON  = 0x37
    DOT    = 0x38
    COMMA  = 0x39
    LARROW = 0x3A
    RARROW = 0x3B
    SEMI   = 0x3C
    EOF    = 0x3F
    LET    = 0x40
    FUNC   = 0x41
    RETURN = 0x42
    STRUCT = 0x43
    PRINT  = 0x80

class Token(object):
    def __init__(self, token_type, token_value, line):
        # token types listed at top of file
        self.type = token_type
        # token values: e.g. 1.0, 1, True, None, etc
        self.value = token_value
        self.line = line
    def __str__(self):
        return "Token(%s, %s)" % (self.type, repr(self.value))
    __repr__ = __str__
