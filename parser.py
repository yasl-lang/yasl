from tokens import *
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
        #print(self.current_token, self.next_token)
        if self.current_token.type == token_type:
            result = self.current_token
            self.current_token = self.next_token
            self.next_token = self.lexer.get_next_token()
            return result
        else:
            self.error(token_type)
    def program(self):
        if self.current_token.type == PRINT:
            token = self.eat(PRINT)
            return Print(token, self.expr())
        elif self.current_token.type == VAR:
            self.eat(VAR)
            return self.vardecl()
        return self.expr()
    def vardecl(self):
        name = self.eat(ID)
        if self.current_token.value == "=":
            self.eat(OP)
        else:
            assert False
        return Decl(name, self.expr())
    def expr(self):
        """if self.next_token.value in [":=", "||=", "*=", "+=", "-="]:
            return self.asgn()
        """
        curr = self.fact0()
        """if self.current_token.value == "?":
            self.eat(QMARK)
            first = self.expr()
            self.eat(COLON)
            second = self.expr()
            return TriOp(Token(OP, "?:"), curr, first, second)
        """
        return curr
    def fact0(self):
        curr = self.fact1()
        """while self.current_token.value in ["&", "|"]:
            op = self.current_token
            self.eat(OP)
            curr = BinOp(op, curr, self.fact1())"""
        return curr
    def fact1(self):
        curr = self.fact2()
        """"while self.current_token.value in ["=", "<>"]:
            op = self.current_token
            self.eat(OP)
            curr = BinOp(op, curr, self.fact2())"""
        return curr
    def fact2(self):
        curr = self.fact3()
        """while self.current_token.value in ["<", ">"]:
            op = self.current_token
            self.eat(OP)
            curr = BinOp(op, curr, self.fact3())"""
        return curr
    def fact3(self):
        curr = self.fact4()
        while self.current_token.value in ["+", "-", "||"]:
            op = self.eat(OP)
            curr = BinOp(op, curr, self.fact4())
        return curr
    def fact4(self):
        curr = self.const()
        while self.current_token.value in ["*", "/"]:
            op = self.eat(OP)
            curr = BinOp(op, curr, self.const())
        return curr
    def const(self):
        if self.current_token.type == OP and self.current_token.value in ["-", "+", "!"]:
            op = self.eat(OP)
            return UnOp(op, self.const())
        elif self.current_token.type == LPAREN:
            self.eat(LPAREN)
            expr = self.expr()
            self.eat(RPAREN)
            return expr
            '''
        elif self.next_token.type == LPAREN:
            return self.func_call()'''
        elif self.current_token.type == ID:
            var = self.eat(ID)
            return Var(var)
            '''
        elif self.current_token.type == STR:
            string = self.current_token
            self.eat(STR)
            return String(string)'''
        elif self.current_token.type == INT:
            integer = self.eat(INT)
            return Integer(integer)
        elif self.current_token.type == FLOAT:
            double = self.eat(FLOAT)
            return Float(double)
            '''
        elif self.current_token.type == BOOL:
            boolean = self.current_token
            self.eat(BOOL)
            return Boolean(boolean)'''
    def parse(self):
        statements = []
        while self.current_token.type != EOF:
            statements.append(self.program())
            if self.current_token.type == SEMI:
                self.eat(SEMI)
            elif self.current_token.type != EOF:
                self.error(EOF)
        return statements
