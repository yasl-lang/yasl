cmake_minimum_required(VERSION 3.9.2)
project(YASL C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
#set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic -Werror")
set(CMAKE_VERBOSE_MAKEFILE OFF)

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
include_directories(interpreter/float)
include_directories(interpreter/integer)
include_directories(interpreter/list)
include_directories(interpreter/table)
include_directories(interpreter/VM)
include_directories(interpreter/YASL_Object)
include_directories(interpreter/YASL_string)
include_directories(interpreter/refcount)
include_directories(interpreter/userdata)
include_directories(std-io)
include_directories(prime)
include_directories(test)

add_executable(YASL
        yasl.c
        main.c
        compiler/ast/ast.c
        bytebuffer/bytebuffer.c
        compiler/compiler/compiler.c
        compiler/compiler/env/env.c
        compiler/lexer/lexer.c
        compiler/parser/parser.c
        hashtable/hashtable.c
        interpreter/boolean/bool_methods.c
        interpreter/builtins/builtins.c
        interpreter/float/float64_methods.c
        interpreter/float/float64.c
        interpreter/integer/int64_methods.c
        interpreter/list/list.c
        interpreter/list/list_methods.c
        interpreter/table/table_methods.c
        interpreter/VM/VM.c
        interpreter/YASL_Object/YASL_Object.c
        interpreter/refcount/refcount.c
        interpreter/YASL_string/str_methods.c
        interpreter/YASL_string/YASL_string.c
        interpreter/userdata/userdata.c
        prime/prime.c
        std-io/yasl-std-io.c)

add_executable(YASLTEST
        test/yats.c
        test/yasl_test.c
        test/lexertest.c
        test/compilertest.c
        test/binoptest.c
        test/unoptest.c
        test/literaltest.c
        test/iftest.c
        test/whiletest.c
        test/fortest.c
        test/foreachtest.c
        test/comprehensiontest.c
        compiler/lexer/lexer.c
        compiler/compiler/compiler.c
        compiler/parser/parser.c
        compiler/ast/ast.c
        hashtable/hashtable.c
        interpreter/float/float64.c
        interpreter/YASL_Object/YASL_Object.c
        bytebuffer/bytebuffer.c
        interpreter/YASL_string/YASL_string.c
        interpreter/refcount/refcount.c
        interpreter/list/list.c
        interpreter/userdata/userdata.c
        prime/prime.c
        compiler/compiler/env/env.c)

add_library(yaslapi
        yasl.c
        compiler/ast/ast.c
        bytebuffer/bytebuffer.c
        compiler/compiler/compiler.c
        compiler/compiler/env/env.c
        compiler/lexer/lexer.c
        compiler/parser/parser.c
        hashtable/hashtable.c
        interpreter/boolean/bool_methods.c
        interpreter/builtins/builtins.c
        interpreter/float/float64_methods.c
        interpreter/float/float64.c
        interpreter/integer/int64_methods.c
        interpreter/list/list.c
        interpreter/list/list_methods.c
        interpreter/table/table_methods.c
        interpreter/VM/VM.c
        interpreter/YASL_Object/YASL_Object.c
        interpreter/refcount/refcount.c
        interpreter/YASL_string/str_methods.c
        interpreter/YASL_string/YASL_string.c
        interpreter/userdata/userdata.c
        prime/prime.c)

target_link_libraries(YASL m)
target_link_libraries(YASLTEST m)
target_link_libraries(yaslapi m)
