from tokens import TokenTypes, Token

###############################################################################
#                                                                             #
#   LEXER                                                                     #
#                                                                             #
###############################################################################

RESERVED_KEYWORDS = {
            "var": lambda x: Token(TokenTypes.VAR, "var", x),
            "print": lambda x: Token(TokenTypes.PRINT, "print", x),
            "if": lambda x: Token(TokenTypes.IF, "if", x),
            "else": lambda x: Token(TokenTypes.ELSE, "else", x),
            "elseif": lambda x: Token(TokenTypes.ELSEIF, "elseif", x),
            "true": lambda x: Token(TokenTypes.BOOL, True, x),
            "false": lambda x: Token(TokenTypes.BOOL, False, x),
            "and": lambda x: Token(TokenTypes.LOGIC, "and", x),
            "or": lambda x: Token(TokenTypes.LOGIC, "or", x),
            "nil": lambda x: Token(TokenTypes.NIL, None, x),
}

class Lexer(object):
    def __init__(self, text):
        # string input
        self.text = text
        # self.pos is index into self.text
        self.pos = 0
        # current line
        self.line = 1
        # current char
        self.current_char = self.text[0]
        # tokens so far
        self.tokens = []
    def error(self):
        raise Exception("LEXING ERROR: line %s" % self.line)
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
        if self.current_char == "\n" and self.tokens[-1].type is TokenTypes.VAR:
            self._add_token(TokenTypes.SEMI)
        elif self.current_char == "\n" and self.tokens[-1].type is TokenTypes.BOOL:
            self._add_token(TokenTypes.SEMI)
        elif self.current_char == "\n" and self.tokens[-1].type is TokenTypes.NIL:
            self._add_token(TokenTypes.SEMI)
        elif self.current_char == "\n" and self.tokens[-1].type is TokenTypes.ID:
            self._add_token(TokenTypes.SEMI)
    '''def _str(self):
        result = ""
        while self.current_char is not None and self.current_char != '"':
            result += self.current_char
            self.advance()
        return result'''
    def _num(self):
        result = ""
        while self.current_char is not None and (self.current_char.isdigit() or self.current_char == "_"):
            result += self.current_char
            self.advance()
        if self.current_char == ".":
            result += self.current_char
            self.advance()
            while self.current_char is not None and (self.current_char.isdigit() or self.current_char == "_"):
                result += self.current_char
                self.advance()
            token = Token(TokenTypes.FLOAT, float(result.replace("_", "")), self.line)
        else:
            token =  Token(TokenTypes.INT, int(result.replace("_", "")), self.line)
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
            elif self.current_char == "#":
                while self.current_char != "\n":
                    self.advance()
            elif self.current_char.isdigit():
                self._num()
            elif self.current_char.isalnum():
                self._id()
            elif self.current_char == ":": self._add_token(TokenTypes.COLON)
            elif self.current_char == "?": self._add_token(TokenTypes.QMARK)
            elif self.current_char == ";": self._add_token(TokenTypes.SEMI)
            elif self.current_char == "(": self._add_token(TokenTypes.LPAREN)
            elif self.current_char == ")":
                self._add_token(TokenTypes.RPAREN)
                self._eat_white_space()
                if self.current_char == "\n":
                    self._add_token(TokenTypes.SEMI)
            elif self.current_char == "{": self._add_token(TokenTypes.LBRACE)
            elif self.current_char == "}":
                self._add_token(TokenTypes.RBRACE)
                self._eat_white_space()
                if self.current_char == "\n":
                    self._add_token(TokenTypes.SEMI)
            elif self.current_char in ("=", "<", ">", "+", "-", "/", "*", "!"): self._add_token(TokenTypes.OP)
            elif self.current_char == "<" and self.peek() == "=":
                self.tokens.append(Token(TokenTypes.OP, "<=", self.line))
                self.advance()
                self.advance()
            elif self.current_char == ">" and self.peek() == "=":
                self.tokens.append(Token(TokenTypes.OP, ">=", self.line))
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
            else:
                self.error()
        #for token in self.tokens: print(token)
        return self.tokens
