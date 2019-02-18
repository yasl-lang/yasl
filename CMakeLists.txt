cmake_minimum_required(VERSION 3.7.2)
project(YASL C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-result -Wno-unused-variable -Wno-unused-parameter -pedantic -Werror")
set(CMAKE_VERBOSE_MAKEFILE OFF)

include_directories(.)
include_directories(std-io)
include_directories(std-math)
include_directories(test)

add_executable(YASL
        yasl.c
        main.c
        compiler/ast.c
        bytebuffer/bytebuffer.c
        compiler/compiler.c
        compiler/env.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/parser.c
        compiler/middleend.c
        hashtable/hashtable.c
        interpreter/bool_methods.c
        interpreter/builtins.c
        interpreter/float_methods.c
        interpreter/yasl_float.c
        interpreter/int_methods.c
        interpreter/list.c
        interpreter/list_methods.c
        interpreter/table_methods.c
        interpreter/VM.c
        interpreter/YASL_Object.c
        interpreter/refcount.c
        interpreter/str_methods.c
        interpreter/YASL_string.c
        interpreter/userdata.c
        interpreter/undef_methods.c
        prime/prime.c
        std-io/yasl-std-io.c
        std-math/yasl-std-math.c)

add_executable(YASLTEST
        test/test_compiler/yats.c
        test/yasl_test.c
        test/test_lexer/lexertest.c
        test/test_compiler/compilertest.c
        test/test_compiler/binoptest.c
        test/test_compiler/unoptest.c
        test/test_compiler/literaltest.c
        test/test_compiler/iftest.c
        test/test_compiler/whiletest.c
        test/test_compiler/fortest.c
        test/test_compiler/foreachtest.c
        test/test_compiler/foldingtest.c
        test/test_compiler/comprehensiontest.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/compiler.c
        compiler/parser.c
        compiler/ast.c
        compiler/middleend.c
        hashtable/hashtable.c
        interpreter/yasl_float.c
        interpreter/YASL_Object.c
        bytebuffer/bytebuffer.c
        interpreter/YASL_string.c
        interpreter/refcount.c
        interpreter/list.c
        interpreter/userdata.c
        prime/prime.c
        compiler/env.c)

add_library(yaslapi
        yasl.c
        compiler/ast.c
        bytebuffer/bytebuffer.c
        compiler/compiler.c
        compiler/env.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/parser.c
        compiler/middleend.c
        hashtable/hashtable.c
        interpreter/bool_methods.c
        interpreter/builtins.c
        interpreter/float_methods.c
        interpreter/yasl_float.c
        interpreter/int_methods.c
        interpreter/list.c
        interpreter/list_methods.c
        interpreter/table_methods.c
        interpreter/VM.c
        interpreter/YASL_Object.c
        interpreter/refcount.c
        interpreter/str_methods.c
        interpreter/YASL_string.c
        interpreter/userdata.c
        interpreter/undef_methods.c
        prime/prime.c)

target_link_libraries(YASL m)
target_link_libraries(YASLTEST m)
target_link_libraries(yaslapi m)
