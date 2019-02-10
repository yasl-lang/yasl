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
        compiler/ast/ast.c
        bytebuffer/bytebuffer.c
        compiler/compiler/compiler.c
        compiler/compiler/env/env.c
        compiler/lexer/lexer.c
        compiler/lexer/lexinput.c
        compiler/parser/parser.c
        compiler/middleend/middleend.c
        hashtable/hashtable.c
        interpreter/boolean/bool_methods.c
        interpreter/builtins/builtins.c
        interpreter/float/float_methods.c
        interpreter/float/yasl_float.c
        interpreter/integer/int_methods.c
        interpreter/list/list.c
        interpreter/list/list_methods.c
        interpreter/table/table_methods.c
        interpreter/VM/VM.c
        interpreter/YASL_Object/YASL_Object.c
        interpreter/refcount/refcount.c
        interpreter/YASL_string/str_methods.c
        interpreter/YASL_string/YASL_string.c
        interpreter/userdata/userdata.c
        interpreter/undef/undef_methods.c
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
        compiler/lexer/lexer.c
        compiler/lexer/lexinput.c
        compiler/compiler/compiler.c
        compiler/parser/parser.c
        compiler/ast/ast.c
        compiler/middleend/middleend.c
        hashtable/hashtable.c
        interpreter/float/yasl_float.c
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
        compiler/lexer/lexinput.c
        compiler/parser/parser.c
        compiler/middleend/middleend.c
        hashtable/hashtable.c
        interpreter/boolean/bool_methods.c
        interpreter/builtins/builtins.c
        interpreter/float/float_methods.c
        interpreter/float/yasl_float.c
        interpreter/integer/int_methods.c
        interpreter/list/list.c
        interpreter/list/list_methods.c
        interpreter/table/table_methods.c
        interpreter/VM/VM.c
        interpreter/YASL_Object/YASL_Object.c
        interpreter/refcount/refcount.c
        interpreter/YASL_string/str_methods.c
        interpreter/YASL_string/YASL_string.c
        interpreter/userdata/userdata.c
        interpreter/undef/undef_methods.c
        prime/prime.c)

target_link_libraries(YASL m)
target_link_libraries(YASLTEST m)
target_link_libraries(yaslapi m)
