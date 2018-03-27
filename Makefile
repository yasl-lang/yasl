CC=gcc
CFLAGS=-lm
OUT=YASL
OBJECTS=interpreter/VM.o interpreter/builtins/builtins.o interpreter/constant/constant.o interpreter/hashtable/hashtable.o interpreter/list/list.o interpreter/prime/prime.o interpreter/string8/string8.o interpreter/vtable/vtable.o

YASL: $(OBJECTS)
	$(CC) interpreter/main.c $(OBJECTS) $(CFLAGS) -o $(OUT)

clean:
	rm $(OUT) $(OBJECTS)

interpreter/VM.o:
	$(CC) interpreter/VM.c -c -o interpreter/VM.o

interpreter/builtins/builtins.o:
	$(CC) interpreter/builtins/builtins.c -c -o interpreter/builtins/builtins.o

interpreter/constant/constant.o:
	$(CC) interpreter/constant/constant.c -c -o interpreter/constant/constant.o

interpreter/hashtable/hashtable.o:
	$(CC) interpreter/hashtable/hashtable.c -c -o interpreter/hashtable/hashtable.o

interpreter/list/list.o:
	$(CC) interpreter/list/list.c -c -o interpreter/list/list.o

interpreter/prime/prime.o:
	$(CC) interpreter/prime/prime.c -c -o interpreter/prime/prime.o

interpreter/string8/string8.o:
	$(CC) interpreter/string8/string8.c -c -o interpreter/string8/string8.o

interpreter/vtable/vtable.o:
	$(CC) interpreter/vtable/vtable.c -c -o interpreter/vtable/vtable.o
