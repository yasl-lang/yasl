from .tokens import TokenTypes, Token
from .ast import *

###############################################################################
#                                                                             #
#  PARSER                                                                     #
#                                                                             #
###############################################################################

class Parser(object):
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
        self.current_token = self.tokens[0]
        #self.next_token = self.self.tokens[1]
    def advance(self):
        self.pos += 1
        if self.pos < len(self.tokens):
            self.current_token = self.tokens[self.pos]
        else:
            self.current_token = Token(TokenTypes.EOF, None, self.tokens[-1].line)
        #self.next_token = self.tokens[self.pos]
    def error(self,token_type=None):
        raise Exception("Expected %s Token in line %s, got %s" % \
            (token_type, self.current_token.line, self.current_token.type))
    def eat(self, token_type):
        print(self.current_token)
        if self.current_token.type is token_type:
            result = self.current_token
            self.advance()
            return result
        else:
            self.error(token_type)
    def program(self):
        if self.current_token.type is  TokenTypes.PRINT:
            token = self.eat(TokenTypes.PRINT)
            return Print(token, self.expr())
        elif self.current_token.type is TokenTypes.IF:
            return self.if_stmt()
        elif self.current_token.type is TokenTypes.WHILE:
            return self.while_loop()
        elif self.current_token.type is TokenTypes.FUNC:
            self.eat(TokenTypes.FUNC)
            return self.fndecl()
        elif self.current_token.type is TokenTypes.RETURN:
            return self.return_stmt()
        elif self.current_token.type is TokenTypes.LET:
            self.eat(TokenTypes.LET)
            return self.vardecl()
        return ExprStmt(self.expr())
    def if_stmt(self):
        if self.current_token.type is TokenTypes.IF:
            token = self.eat(TokenTypes.IF)
        else:
            token = self.eat(TokenTypes.ELSEIF)
        cond = self.expr()
        self.eat(TokenTypes.LBRACE)
        body = []
        while self.current_token.type is not TokenTypes.RBRACE:
            body.append(self.program())
            self.eat(TokenTypes.SEMI)
        self.eat(TokenTypes.RBRACE)
        if self.current_token.type is not TokenTypes.ELSE and self.current_token.type is not TokenTypes.ELSEIF:
            return If(token, cond, Block(body))
        if self.current_token.type is TokenTypes.SEMI:
            self.eat(TokenTypes.SEMI)
        if self.current_token.type is TokenTypes.ELSEIF:
            return IfElse(token, cond, Block(body), self.if_stmt())
        if self.current_token.type is TokenTypes.ELSE:
            left = body
            right = []
            self.eat(TokenTypes.ELSE)
            self.eat(TokenTypes.LBRACE)
            while self.current_token.type is not TokenTypes.RBRACE:
                right.append(self.program())
                self.eat(TokenTypes.SEMI)
            self.eat(TokenTypes.RBRACE)
            return IfElse(token, cond, Block(left), Block(right))
        assert False
    def while_loop(self):
        token = self.eat(TokenTypes.WHILE)
        cond = self.expr()
        self.eat(TokenTypes.LBRACE)
        body = []
        while self.current_token.type is not TokenTypes.RBRACE:
            body.append(self.program())
            self.eat(TokenTypes.SEMI)
        self.eat(TokenTypes.RBRACE)
        return While(token, cond, Block(body))
    def vardecl(self):
        name = self.eat(TokenTypes.ID)
        if self.current_token.value == "=":
            self.eat(TokenTypes.OP)
            return Decl(name, self.expr())
        else:
            return Decl(name, Undef(Token(TokenTypes.UNDEF, None, name.line)))
    def fndecl(self):
        name = self.eat(TokenTypes.ID)
        self.eat(TokenTypes.COLON)
        params = []
        if self.current_token.type is not TokenTypes.ARROW:
            params.append(self.eat(TokenTypes.ID))
        while self.current_token.type is TokenTypes.COMMA:
            self.eat(TokenTypes.COMMA)
            params.append(self.eat(TokenTypes.ID))
        self.eat(TokenTypes.ARROW)
        block = []
        self.eat(TokenTypes.LBRACE)
        while self.current_token.type is not TokenTypes.RBRACE:
            block.append(self.program())
            self.eat(TokenTypes.SEMI)
        self.eat(TokenTypes.RBRACE)
        return FunctionDecl(name, params, Block(block))
    def func_call(self):
        self.eat(TokenTypes.LPAREN)
        params = []
        if self.current_token.type is not TokenTypes.RPAREN:
            params.append(self.expr())
        while self.current_token.type is TokenTypes.COMMA:
            self.eat(TokenTypes.COMMA)
            params.append(self.expr())
        self.eat(TokenTypes.RPAREN)
        return params
    def return_stmt(self):
        token = self.eat(TokenTypes.RETURN)
        return Return(token, self.expr())
    def expr(self):
        return self.assign()
    def assign(self):
        name = self.ternary()
        if self.current_token.value == "=":
            self.eat(TokenTypes.OP)
            if isinstance(name, Var) or isinstance(name, Index):
                right = self.expr()
                return Assign(name, right)
            else:
                raise Exception("Invalid assignment target.")
        else:
            return name
    def ternary(self):
        """if self.next_token.value in [":=", "||=", "*=", "+=", "-="]:
            return self.asgn()
        """
        curr = self.logic_or()
        if self.current_token.value == "?":
            token = self.eat(TokenTypes.QMARK)
            first = self.expr()
            self.eat(TokenTypes.COLON)
            second = self.expr()
            return TriOp(Token(TokenTypes.OP, "?:", token.line), curr, first, second)
        elif self.current_token.value == "??":
            token = self.eat(TokenTypes.OP)
            return BinOp(token, curr, self.expr())
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
        curr = self.comparator()
        while self.current_token.value in ["==", "!=", "===", "!=="]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.comparator())
        return curr
    def comparator(self):
        curr = self.concat()
        while self.current_token.value in ["<", ">", ">=", "<="]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.add())
        return curr
    def concat(self):
        curr = self.add()
        if self.current_token.value == "||":
            token = self.eat(TokenTypes.OP)
            return BinOp(token, curr, self.concat())
        return curr
    def add(self):
        curr = self.multiply()
        while self.current_token.value in ["+", "-"]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.multiply())
        return curr
    def multiply(self):
        curr = self.unop()
        while self.current_token.value in ["*", "/", "%"]:
            op = self.eat(TokenTypes.OP)
            curr = BinOp(op, curr, self.const())
        return curr
    def unop(self):
        if self.current_token.type is TokenTypes.OP and self.current_token.value in ("-", "+", "!", "#"):
            op = self.eat(TokenTypes.OP)
            return UnOp(op, self.unop())
        else:
            return self.const()
    def const(self):
        #if self.current_token.type is TokenTypes.OP and self.current_token.value in ("-", "+", "!", "#"):
        #    op = self.eat(TokenTypes.OP)
        #    return UnOp(op, self.const())
        result = self.literal()
        while self.current_token.type is TokenTypes.DOT:
            self.eat(TokenTypes.DOT)
            right = self.literal()
            if isinstance(right, FunctionCall):
                return MethodCall(result, right.token, right.params)
            else:
                assert False
        while self.current_token.type is TokenTypes.LBRACK:
            self.eat(TokenTypes.LBRACK)
            result = Index(result, self.expr())
            self.eat(TokenTypes.RBRACK)
        return result
    def literal(self):
        if self.current_token.type is TokenTypes.LPAREN:
            self.eat(TokenTypes.LPAREN)
            expr = self.expr()
            self.eat(TokenTypes.RPAREN)
            return expr
        elif self.current_token.type is TokenTypes.ID:
            var = self.eat(TokenTypes.ID)
            if self.current_token.type is TokenTypes.LPAREN:
                    left = var
                    right = self.func_call()
                    return FunctionCall(left, right)
                    '''elif self.current_token.type is TokenTypes.LBRACK:
                result = Var(var)
                while self.current_token.type is TokenTypes.LBRACK:
                    self.eat(TokenTypes.LBRACK)
                    result = Index(result, self.expr())
                    self.eat(TokenTypes.RBRACK)
                return result'''
                    '''elif self.current_token.type is TokenTypes.DOT:
                result = Var(var)
                while self.current_token.type is TokenTypes.DOT:
                    self.eat(TokenTypes.DOT)
                    result = MemberAccess(result, self.const())
                return result'''
            else:
                return Var(var)
        elif self.current_token.type is TokenTypes.STR:
            string = self.eat(TokenTypes.STR)
            return String(string)
        elif self.current_token.type is TokenTypes.INT:
            integer = self.eat(TokenTypes.INT)
            return Integer(integer)
        elif self.current_token.type is TokenTypes.FLOAT:
            double = self.eat(TokenTypes.FLOAT)
            return Float(double)
        elif self.current_token.type is TokenTypes.BOOL:
            boolean = self.eat(TokenTypes.BOOL)
            return Boolean(boolean)
        elif self.current_token.type is TokenTypes.UNDEF:
            nil = self.eat(TokenTypes.UNDEF)
            return Undef(nil)
        elif self.current_token.type is TokenTypes.MAP:
            hash = self.eat(TokenTypes.MAP)
            self.eat(TokenTypes.LPAREN)
            keys = []
            vals = []
            if self.current_token.type is not TokenTypes.RPAREN:
                keys.append(self.expr())
                self.eat(TokenTypes.ARROW)
                vals.append(self.expr())
            while self.current_token.type is TokenTypes.COMMA and self.current_token.type is not TokenTypes.EOF:
                self.eat(TokenTypes.COMMA)
                keys.append(self.expr())
                self.eat(TokenTypes.ARROW)
                vals.append(self.expr())
            self.eat(TokenTypes.RPAREN)
            return Hash(hash, keys, vals)
        elif self.current_token.type is TokenTypes.LBRACK:
            ls = self.eat(TokenTypes.LBRACK)
            keys = []
            vals = []
            if self.current_token.type is not TokenTypes.RBRACK:
                keys.append(self.expr())
                if self.current_token.type is TokenTypes.ARROW:
                    self.eat(TokenTypes.ARROW)
                    vals.append(self.expr())
                    while self.current_token.type is TokenTypes.COMMA and self.current_token.type is not TokenTypes.EOF:
                        self.eat(TokenTypes.COMMA)
                        keys.append(self.expr())
                        self.eat(TokenTypes.ARROW)
                        vals.append(self.expr())
                    self.eat(TokenTypes.RBRACK)
                    return Hash(ls, keys, vals)
                else:
                    while self.current_token.type is TokenTypes.COMMA and self.current_token.type is not TokenTypes.EOF:
                        self.eat(TokenTypes.COMMA)
                        keys.append(self.expr())
                    self.eat(TokenTypes.RBRACK)
                    return List(ls, keys)
            return List(ls, keys) #TODO: decide way to make empty map
            '''elif self.current_token.type is TokenTypes.LIST:
            ls = self.eat(TokenTypes.LIST)
            self.eat(TokenTypes.LPAREN)
            params = []
            if self.current_token.type is not TokenTypes.RPAREN:
                params.append(self.expr())
            while self.current_token.type is TokenTypes.COMMA and self.current_token.type is not TokenTypes.EOF:
                self.eat(TokenTypes.COMMA)
                params.append(self.expr())
            self.eat(TokenTypes.RPAREN)
            return List(ls, params)'''
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
        #for statement in statements: print(statement)
        return statements