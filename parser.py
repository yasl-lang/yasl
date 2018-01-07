from tokens import TokenTypes, Token
from ast import *

###############################################################################
#                                                                             #
#  PARSER                                                                     #
#                                                                             #
###############################################################################

class Parser(object):
    def __init__(self, lexer):
        self.lexer = lexer
        self.current_token = self.lexer.get_next_token()
        self.next_token = self.lexer.get_next_token()
    def error(self,token_type=None):
        raise Exception("Expected %s Token at pos %s, got %s" % \
            (token_type, self.lexer.pos, self.current_token))
    def eat(self, token_type):
        print(self.current_token)
        if self.current_token.type is token_type:
            result = self.current_token
            self.current_token = self.next_token
            self.next_token = self.lexer.get_next_token()
            return result
        else:
            self.error(token_type)
    def program(self):
        if self.current_token.type is  TokenTypes.PRINT:
            token = self.eat(TokenTypes.PRINT)
            return Print(token, self.expr())
        elif self.current_token.type is TokenTypes.IF:
            return self.if_stmt()
        elif self.current_token.type is TokenTypes.VAR:
            self.eat(TokenTypes.VAR)
            return self.vardecl()
        return self.expr()
    def if_stmt(self):
        token = self.eat(TokenTypes.IF)
        cond = self.expr()
        self.eat(TokenTypes.LBRACE)
        self.eat(TokenTypes.SEMI)
        body = []
        while self.current_token.type is not TokenTypes.RBRACE:
            body.append(self.program())
            self.eat(TokenTypes.SEMI)
        self.eat(TokenTypes.RBRACE)
        return If(token, cond, Block(body))
    def vardecl(self):
        name = self.eat(TokenTypes.ID)
        if self.current_token.value == "=":
            self.eat(TokenTypes.OP)
        else:
            assert False
        return Decl(name, self.expr())
    def expr(self):
        """if self.next_token.value in [":=", "||=", "*=", "+=", "-="]:
            return self.asgn()
        """
        curr = self.logic_or()
        if self.current_token.value == "?":
            self.eat(TokenTypes.QMARK)
            first = self.expr()
            self.eat(TokenTypes.COLON)
            second = self.expr()
            return TriOp(Token(TokenTypes.OP, "?:"), curr, first, second)
        return curr
    def logic_or(self):
        curr = self.logic_and()
        while self.current_token.value == "or":
            op = self.eat(TokenTypes.LOGIC)
            curr = LogicOp(op, curr, self.logic_and())
        return curr
    def logic_and(self):
        curr = self.fact0()
        while self.current_token.value == "and":
            op = self.eat(TokenTypes.LOGIC)
            curr = LogicOp(op, curr, self.fact0())
        return curr
    def fact0(self):
        curr = self.fact1()
        """while self.current_token.value in ["&", "|"]:
            op = self.current_token
            self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.fact1())"""
        return curr
    def fact1(self):
        curr = self.fact2()
        while self.current_token.value in ["==", "!="]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.fact2())
        return curr
    def fact2(self):
        curr = self.fact3()
        while self.current_token.value in ["<", ">", ">=", "<="]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.fact3())
        return curr
    def fact3(self):
        curr = self.fact4()
        while self.current_token.value in ["+", "-", "||"]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.fact4())
        return curr
    def fact4(self):
        curr = self.const()
        while self.current_token.value in ["*", "/"]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.const())
        return curr
    def const(self):
        if self.current_token.type is TokenTypes.OP and self.current_token.value in ["-", "+", "!"]:
            op = self.eat(TokenTypes.OP)
            return UnOp(op, self.const())
        elif self.current_token.type is TokenTypes.LPAREN:
            self.eat(TokenTypes.LPAREN)
            expr = self.expr()
            self.eat(TokenTypes.RPAREN)
            return expr
            '''
        elif self.next_token.type == LPAREN:
            return self.func_call()'''
        elif self.current_token.type is TokenTypes.ID:
            var = self.eat(TokenTypes.ID)
            return Var(var)
            '''
        elif self.current_token.type == STR:
            string = self.current_token
            self.eat(TokenTypes.STR)
            return String(string)'''
        elif self.current_token.type is TokenTypes.INT:
            integer = self.eat(TokenTypes.INT)
            return Integer(integer)
        elif self.current_token.type is TokenTypes.FLOAT:
            double = self.eat(TokenTypes.FLOAT)
            return Float(double)
        elif self.current_token.type is TokenTypes.BOOL:
            boolean = self.eat(TokenTypes.BOOL)
            return Boolean(boolean)
        elif self.current_token.type is TokenTypes.NIL:
            nil = self.eat(TokenTypes.NIL)
            return Nil(nil)
        else:
            assert False
    def parse(self):
        statements = []
        while self.current_token.type is not TokenTypes.EOF:
            statements.append(self.program())
            if self.current_token.type is TokenTypes.SEMI:
                self.eat(TokenTypes.SEMI)
            elif self.current_token.type is not TokenTypes.EOF:
                self.error(TokenTypes.EOF)
        print(statements)
        return statements