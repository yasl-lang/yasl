cmake_minimum_required(VERSION 3.10)
project(YASL C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(.)
include_directories(compiler)
include_directories(compiler/ast)
include_directories(bytebuffer)
include_directories(compiler/compiler)
include_directories(compiler/compiler/env)
include_directories(compiler/lexer)
include_directories(compiler/parser)
include_directories(hashtable)
include_directories(interpreter/boolean)
include_directories(interpreter/builtins)
include_directories(interpreter/file)
include_directories(interpreter/float)
include_directories(interpreter/integer)
include_directories(interpreter/list)
include_directories(interpreter/table)
include_directories(interpreter/VM)
include_directories(interpreter/YASL_Object)
include_directories(interpreter/YASL_string)
include_directories(prime)
include_directories(test)

add_executable(YASL
        main.c
        compiler/ast/ast.c
        compiler/ast/ast.h
        bytebuffer/bytebuffer.c
        bytebuffer/bytebuffer.h
        compiler/compiler/compiler.c
        compiler/compiler/compiler.h
        compiler/compiler/env/env.c
        compiler/compiler/env/env.h
        compiler/lexer/lexer.c
        compiler/lexer/lexer.h
        compiler/parser/parser.c
        compiler/parser/parser.h
        compiler/token.h
        hashtable/hashtable.c
        hashtable/hashtable.h
        interpreter/boolean/bool_methods.c
        interpreter/boolean/bool_methods.h
        interpreter/builtins/builtins.c
        interpreter/builtins/builtins.h
        interpreter/file/file_methods.c
        interpreter/file/file_methods.h
        interpreter/float/float64_methods.c
        interpreter/float/float64_methods.h
        interpreter/integer/int64_methods.c
        interpreter/integer/int64_methods.h
        interpreter/list/list.c
        interpreter/list/list.h
        interpreter/list/list_methods.c
        interpreter/list/list_methods.h
        interpreter/table/table_methods.c
        interpreter/table/table_methods.h
        interpreter/VM/VM.c
        interpreter/VM/VM.h
        interpreter/YASL_Object/YASL_Object.c
        interpreter/YASL_Object/YASL_Object.h
        interpreter/YASL_string/str_methods.c
        interpreter/YASL_string/str_methods.h
        interpreter/YASL_string/YASL_string.c
        interpreter/YASL_string/YASL_string.h
        interpreter/YASL_string/refcountptr.c
        interpreter/YASL_string/refcountptr.h
        prime/prime.c
        prime/prime.h
        functions.h
        opcode.h)

add_executable(YASLTEST
        test/lexertest.c
        compiler/lexer/lexer.c
        compiler/lexer/lexer.h)

target_link_libraries(YASL m)