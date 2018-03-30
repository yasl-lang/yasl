from enum import Enum

def auto():
    num = 0
    while True:
        yield num
        num += 1

class TokenTypes(Enum):
    INT    = auto()
    FLOAT  = auto()
    BOOL   = auto()
    UNDEF  = auto()
    STR    = auto()
    MAP    = auto()
    LIST   = auto()
    IF     = auto()
    ELSEIF = auto()
    ELSE   = auto()
    WHILE  = auto()
    FOR    = auto()
    OP     = auto()
    LOGIC  = auto()
    ID     = auto()
    LPAREN = auto()
    RPAREN = auto()
    LBRACK = auto()
    RBRACK = auto()
    LBRACE = auto()
    RBRACE = auto()
    QMARK  = auto()
    COLON  = auto()
    DOT    = auto()
    COMMA  = auto()
    LARROW = auto()
    RARROW = auto()
    SEMI   = auto()
    EOF    = auto()
    LET    = auto()
    FUNC   = auto()
    RETURN = auto()
    STRUCT = auto()
    PRINT  = auto()
    INPUT  = auto()

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
