from .tokens import TokenTypes, Token
from string import hexdigits

###############################################################################
#                                                                             #
#   LEXER                                                                     #
#                                                                             #
###############################################################################

ESCCHARS = {
    "n":  "\n",
    "t":  "\t",
    "\\": "\\",
    "\"": "\"",
    "r":  "\r",
}

RESERVED_KEYWORDS = {
            "let": lambda x: Token(TokenTypes.LET, "let", x),
            "print": lambda x: Token(TokenTypes.PRINT, "print", x),
            "input": lambda x: Token(TokenTypes.INPUT, "input", x),
            "if": lambda x: Token(TokenTypes.IF, "if", x),
            "else": lambda x: Token(TokenTypes.ELSE, "else", x),
            "elseif": lambda x: Token(TokenTypes.ELSEIF, "elseif", x),
            "while": lambda x: Token(TokenTypes.WHILE, "while", x),
            "for": lambda x: Token(TokenTypes.FOR, "for", x),
            "true": lambda x: Token(TokenTypes.BOOL, True, x),
            "false": lambda x: Token(TokenTypes.BOOL, False, x),
            "and": lambda x: Token(TokenTypes.LOGIC, "and", x),
            "or": lambda x: Token(TokenTypes.LOGIC, "or", x),
            "undef": lambda x: Token(TokenTypes.UNDEF, None, x),
            "func": lambda x: Token(TokenTypes.FUNC, "func", x),
            "return": lambda x: Token(TokenTypes.RETURN, "return", x),
            "struct": lambda x: Token(TokenTypes.STRUCT, "struct", x),
            "int64": lambda x: Token(TokenTypes.TYPE, "int64", x),
            "float64": lambda x: Token(TokenTypes.TYPE, "float64", x),
            "bool": lambda x: Token(TokenTypes.TYPE, "bool", x),
            "str8": lambda x: Token(TokenTypes.TYPE, "str8", x),
            "list": lambda x: Token(TokenTypes.TYPE, "list", x),
            "map": lambda x: Token(TokenTypes.TYPE, "map", x),
            "file": lambda x: Token(TokenTypes.TYPE, "file", x),
}

class Lexer(object):
    def __init__(self, text):
        self.text = text         # string input
        self.pos = 0             # self.pos is index into self.text
        self.line = 1            # current line
        self.current_char = self.text[0] # current char
        self.tokens = []         # tokens so far
    def error(self, msg=""):
        raise Exception("LexingError, line %s: " + msg + "." % self.line)
    def peek(self, lookahead=1):
        peek_pos = self.pos + lookahead
        if peek_pos >= len(self.text):
            return None
        else:
            return self.text[peek_pos]
    def advance(self):
        self.pos += 1
        if self.pos < len(self.text):
            self.current_char = self.text[self.pos]
            if self.current_char == "\n":
                self.line += 1
        else:
            self.current_char = None
    def _eat_white_space(self):
        while self.current_char == " ": self.advance()
    def _add_token(self, token_type):
        self.tokens.append(Token(token_type, self.current_char, self.line))
        self.advance()
    def _id(self):
        result = ""
        while self.current_char is not None and (self.current_char.isalnum() or self.current_char == "_"):
            result += self.current_char
            self.advance()
        token = RESERVED_KEYWORDS.get(result, lambda x: Token(TokenTypes.ID, result, x))(self.line)
        self.tokens.append(token)
        self._eat_white_space()
        if self.current_char == "\n" and self.tokens[-1].type is TokenTypes.LET:
            self._add_token(TokenTypes.SEMI)
        elif self.current_char == "\n" and self.tokens[-1].type is TokenTypes.BOOL:
            self._add_token(TokenTypes.SEMI)
        elif self.current_char == "\n" and self.tokens[-1].type is TokenTypes.UNDEF:
            self._add_token(TokenTypes.SEMI)
        elif self.current_char == "\n" and self.tokens[-1].type is TokenTypes.ID:
            self._add_token(TokenTypes.SEMI)
    def _str(self):
        result = ""
        while self.current_char is not None and self.current_char != '"':
            if self.current_char == "\\":
                escape_char = ESCCHARS.get(self.peek(), None)
                if escape_char is None:
                    self.error("invalid escape sequence")
                result += escape_char
                self.advance()
                self.advance()
            else:
                result += self.current_char
                self.advance()
        self.advance()
        token = Token(TokenTypes.STR, result, self.line)
        self.tokens.append(token)
        self._eat_white_space()
        if self.current_char == "\n":
            self._add_token(TokenTypes.SEMI)
    def _hex(self):
        result = "0x"
        self.advance()
        self.advance()
        while self.current_char is not None and (self.current_char in hexdigits or self.current_char == "_"):
            result += self.current_char
            self.advance()
        if result.__len__() < 3:
            self.error("invalid hex literal.")
        token = Token(TokenTypes.INT, int(result.replace("_", ""), 16), self.line)
        self.tokens.append(token)
        self._eat_white_space()
        if self.current_char == "\n":
            self._add_token(TokenTypes.SEMI)
    def _num(self):
        result = ""
        while self.current_char is not None and (self.current_char.isdigit() or self.current_char == "_"):
            result += self.current_char
            self.advance()
        if self.current_char == ".":
            if self.peek().isdigit():
                result += self.current_char
                self.advance()
                while self.current_char is not None and (self.current_char.isdigit() or self.current_char == "_"):
                    result += self.current_char
                    self.advance()
                token = Token(TokenTypes.FLOAT, float(result.replace("_", "")), self.line)
            else:
                self.advance()
                token = Token(TokenTypes.INT, int(result.replace("_", "")), self.line)
                self.tokens.append(token)
                self.tokens.append(Token(TokenTypes.DOT, ".", self.line))
                self._eat_white_space()
                return
        else:
            token = Token(TokenTypes.INT, int(result.replace("_", "")), self.line)
        self.tokens.append(token)
        self._eat_white_space()
        if self.current_char == "\n":
            self._add_token(TokenTypes.SEMI)
    def lex(self):
        # tokenizer
        text = self.text
        while self.pos < len(text):
            while self.current_char in (" ", "\n"):
                self.advance()
            if self.pos >= len(text):
                self.tokens.append(Token(TokenTypes.EOF, None, self.line))
            elif self.current_char == "/" and self.peek() == "$":
                while not (self.current_char == "$" and self.peek() == "/"):
                    self.advance()
                self.advance()
                self.advance()
            elif self.current_char == "$" and self.peek() == "$":
                while self.current_char != "\n" and self.current_char != None:
                    self.advance()
            elif self.current_char == '"':
                self.advance()
                self._str()
            elif self.current_char.isdigit():
                if self.current_char == "0" and self.peek() == "x":
                    self._hex()
                else:
                    self._num()
            elif self.current_char.isalnum():
                self._id()
            elif self.current_char == "?" and self.peek() == "?":
                self.tokens.append(Token(TokenTypes.OP, "??", self.line))
                self.advance()
                self.advance()
            elif self.current_char == "-" and self.peek() == ">":
                self.tokens.append(Token(TokenTypes.RARROW, "->", self.line))
                self.advance()
                self.advance()
            elif self.current_char == "<" and self.peek() == "-":
                self.tokens.append(Token(TokenTypes.LARROW, "<-", self.line))
                self.advance()
                self.advance()
            elif self.current_char == ":": self._add_token(TokenTypes.COLON)
            elif self.current_char == "?": self._add_token(TokenTypes.QMARK)
            elif self.current_char == ";": self._add_token(TokenTypes.SEMI)
            elif self.current_char == "(": self._add_token(TokenTypes.LPAREN)
            elif self.current_char == ")":
                self._add_token(TokenTypes.RPAREN)
                self._eat_white_space()
                if self.current_char == "\n":
                    self._add_token(TokenTypes.SEMI)
            elif self.current_char == "[": self._add_token(TokenTypes.LBRACK)
            elif self.current_char == "]":
                self._add_token(TokenTypes.RBRACK)
                self._eat_white_space()
                if self.current_char == "\n":
                    self._add_token(TokenTypes.SEMI)
            elif self.current_char == "{": self._add_token(TokenTypes.LBRACE)
            elif self.current_char == "}":
                self._add_token(TokenTypes.RBRACE)
                self._eat_white_space()
                if self.current_char == "\n":
                    self._add_token(TokenTypes.SEMI)
            elif self.current_char == "<" and self.peek() == "=":
                self.tokens.append(Token(TokenTypes.OP, "<=", self.line))
                self.advance()
                self.advance()
            elif self.current_char == ">" and self.peek() == "=":
                self.tokens.append(Token(TokenTypes.OP, ">=", self.line))
                self.advance()
                self.advance()
            elif self.current_char == "=" and self.peek(1) == "=" and self.peek(2) == "=":
                self.tokens.append(Token(TokenTypes.OP, "===", self.line))
                self.advance()
                self.advance()
                self.advance()
            elif self.current_char == "!" and self.peek(1) == "=" and self.peek(2) == "=":
                self.tokens.append(Token(TokenTypes.OP, "!==", self.line))
                self.advance()
                self.advance()
                self.advance()
            elif self.current_char == "=" and self.peek() == "=":
                self.tokens.append(Token(TokenTypes.OP, "==", self.line))
                self.advance()
                self.advance()
            elif self.current_char == "!" and self.peek() == "=":
                self.tokens.append(Token(TokenTypes.OP, "!=", self.line))
                self.advance()
                self.advance()
            elif self.current_char == "|" and self.peek() == "|":
                self.tokens.append(Token(TokenTypes.OP, "||", self.line))
                self.advance()
                self.advance()
            elif self.current_char == "/" and self.peek() == "/":
                self.tokens.append(Token(TokenTypes.OP, "//", self.line))
                self.advance()
                self.advance()
            elif self.current_char == ",":
                self._add_token(TokenTypes.COMMA)
            elif self.current_char == ".":
                self._add_token(TokenTypes.DOT)
            elif self.current_char in ("=", "<", ">", "+", "-", "/", "*", "!", "#", "%"): #, "&", "|", "^"):
                self._add_token(TokenTypes.OP)
            else:
                self.error("unknown sequence.")
        return self.tokens
