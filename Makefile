CC=gcc
CFLAGS=-lm -O3 -g
COUT=YASLC
OUT=YASL
OBJECTS=interpreter/VM/VM.o interpreter/builtins/builtins.o interpreter/float/float64_methods.o interpreter/integer/int64_methods.o interpreter/boolean/bool_methods.o interpreter/YASL_string/str_methods.o interpreter/list/list_methods.o interpreter/map/map_methods.o interpreter/file/file_methods.o interpreter/YASL_Object/YASL_Object.o hashtable/hashtable.o interpreter/list/list.o prime/prime.o interpreter/YASL_string/YASL_string.o interpreter/vtable/vtable.o compiler-c/lexer/lexer.o compiler-c/ast/ast.o compiler-c/parser/parser.o compiler-c/compiler/compiler.o compiler-c/bytebuffer/bytebuffer.o compiler-c/env/env.o hashtable/hashtable.o interpreter/YASL_Object/YASL_Object.o prime/prime.o interpreter/list/list.o interpreter/YASL_string/YASL_string.o

YASLC: $(COBJECTS)
	$(CC) $(COBJECTS) compiler-c/main.c $(CFLAGS) -o $(COUT)

YASL: $(OBJECTS)
	$(CC) $(OBJECTS) main.c $(CFLAGS) -o $(OUT)

clean:
	rm $(OUT) $(OBJECTS)

remake: clean YASL