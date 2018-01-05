from lexer import Lexer
from parser import Parser
from interpreter import Interpreter
from resolver import Resolver
import sys
from opcode import HALT

def main():
    interpreter = Interpreter()
    while True:
        try:
            text = input("YASL> ")
        except EOFError:
            break
        if not text:
            continue
        lexer = Lexer(text)
        parser = Parser(lexer)
        statements = parser.parse() # = Interpreter(parser)
        resolver = Resolver(interpreter)
        #resolver.resolve(statements)
        result = interpreter.interpret(statements)
        print([hex(r) for r in result])

if __name__ == "__main__":
    if len(sys.argv) == 1:
        main()
    else:
        assert(len(sys.argv) == 2)
        text = open(sys.argv[1]).read() #TODO: make more general, so file can be located anywhere
        statements = Parser(Lexer(text)).parse()
        interpreter = Interpreter()
        resolver = Resolver(interpreter)
        #resolver.resolve(statements)
        result = interpreter.interpret(statements)
        print(result)
        print([hex(r) for r in result])
        f = open("source.py", "w")
        f.write("from opcode import *\n\n")
        f.write("source = bytes(\n    ")
        f.write(str(result+[HALT]))
        f.write("\n)\n")
        f.close()