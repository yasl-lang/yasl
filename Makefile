CC=gcc
CFLAGS=-lm -O3
COUT=YASLC
OUT=YASL
OBJECTS=interpreter/VM/VM.o interpreter/builtins/builtins.o interpreter/float/float64_methods.o interpreter/integer/int64_methods.o interpreter/boolean/bool_methods.o interpreter/YASL_string/str_methods.o interpreter/list/list_methods.o interpreter/hashtable/hash_methods.o interpreter/file/file_methods.o interpreter/constant/constant.o interpreter/hashtable/hashtable.o interpreter/list/list.o prime/prime.o interpreter/YASL_string/YASL_string.o interpreter/vtable/vtable.o
COBJECTS=compiler-c/lexer/lexer.o compiler-c/ast/ast.o compiler-c/parser/parser.o compiler-c/compiler/compiler.o compiler-c/bytebuffer/bytebuffer.o

YASLC: $(COBJECTS)
	$(CC) $(COBJECTS) compiler-c/main.c $(CFLAGS) -o $(COUT)

YASL: $(OBJECTS)
	$(CC) $(OBJECTS) interpreter/main.c $(CFLAGS) -o $(OUT)

cleanc:
	rm $(COUT) $(COBJECTS)

clean:
	rm $(OUT) $(OBJECTS)

remake: clean YASL

remakec: cleanc YASLC

interpreter/VM/VM.o:
	$(CC) interpreter/VM/VM.c -c -o interpreter/VM/VM.o

interpreter/builtins/builtins.o:
	$(CC) interpreter/builtins/builtins.c $(CFLAGS) -c -o interpreter/builtins/builtins.o

interpreter/float/float64_methods.o:
	$(CC) interpreter/float/float64_methods.c $(CFLAGS) -c -o interpreter/float/float64_methods.o

interpreter/integer/int64_methods.o:
	$(CC) interpreter/integer/int64_methods.c $(CFLAGS) -c -o interpreter/integer/int64_methods.o

interpreter/boolean/bool_methods.o:
	$(CC) interpreter/boolean/bool_methods.c $(CFLAGS) -c -o interpreter/boolean/bool_methods.o

interpreter/YASL_string/str_methods.o:
	$(CC) interpreter/YASL_string/str_methods.c $(CFLAGS) -c -o interpreter/YASL_string/str_methods.o

interpreter/list/list_methods.o:
	$(CC) interpreter/list/list_methods.c $(CFLAGS) -c -o interpreter/list/list_methods.o

interpreter/hashtable/hash_methods.o:
	$(CC) interpreter/hashtable/hash_methods.c $(CFLAGS) -c -o interpreter/hashtable/hash_methods.o

interpreter/file/file_methods.o:
	$(CC) interpreter/file/file_methods.c $(CFLAGS) -c -o interpreter/file/file_methods.o

interpreter/constant/constant.o:
	$(CC) interpreter/constant/constant.c $(CFLAGS) -c -o interpreter/constant/constant.o

interpreter/hashtable/hashtable.o:
	$(CC) interpreter/hashtable/hashtable.c $(CFLAGS) -c -o interpreter/hashtable/hashtable.o

interpreter/list/list.o:
	$(CC) interpreter/list/list.c $(CFLAGS) -c -o interpreter/list/list.o

prime/prime.o:
	$(CC) prime/prime.c $(CFLAGS) -c -o prime/prime.o

interpreter/YASL_string/YASL_string.o:
	$(CC) interpreter/YASL_string/YASL_string.c $(CFLAGS) -c -o interpreter/YASL_string/YASL_string.o

interpreter/vtable/vtable.o:
	$(CC) interpreter/vtable/vtable.c $(CFLAGS) -c -o interpreter/vtable/vtable.o
