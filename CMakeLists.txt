cmake_minimum_required(VERSION 3.5.0)
project(YASL CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-result -Wno-unused-variable -Wno-unused-parameter -Wpedantic -Werror --std=c++2a")
set(CMAKE_VERBOSE_MAKEFILE OFF)

include_directories(.)
include_directories(std-io)
include_directories(std-math)
include_directories(test)

add_executable(YASL
        yasl.cpp
        main.cpp
        compiler/ast.cpp
        bytebuffer/bytebuffer.cpp
        compiler/compiler.cpp
        compiler/env.cpp
        compiler/lexer.cpp
        compiler/lexinput.cpp
        compiler/parser.cpp
        compiler/middleend.cpp
        hashtable/hashtable.cpp
        interpreter/bool_methods.cpp
        interpreter/builtins.cpp
        interpreter/float_methods.cpp
        interpreter/yasl_float.cpp
        interpreter/int_methods.cpp
        interpreter/list.cpp
        interpreter/list_methods.cpp
        interpreter/table_methods.cpp
        interpreter/VM.cpp
        interpreter/YASL_Object.cpp
        interpreter/refcount.cpp
        interpreter/str_methods.cpp
        interpreter/YASL_string.cpp
        interpreter/userdata.cpp
        interpreter/undef_methods.cpp
        prime/prime.cpp
        std-io/yasl-std-io.cpp
        std-math/yasl-std-math.cpp)

add_executable(YASLTEST
        test/test_compiler/yats.cpp
        test/yasl_test.cpp
        test/test_lexer/lexertest.cpp
        test/test_compiler/compilertest.cpp
        test/test_compiler/binoptest.cpp
        test/test_compiler/unoptest.cpp
        test/test_compiler/literaltest.cpp
        test/test_compiler/iftest.cpp
        test/test_compiler/whiletest.cpp
        test/test_compiler/fortest.cpp
        test/test_compiler/foreachtest.cpp
        test/test_compiler/foldingtest.cpp
        test/test_compiler/comprehensiontest.cpp
        compiler/lexer.cpp
        compiler/lexinput.cpp
        compiler/compiler.cpp
        compiler/parser.cpp
        compiler/ast.cpp
        compiler/middleend.cpp
        hashtable/hashtable.cpp
        interpreter/yasl_float.cpp
        interpreter/YASL_Object.cpp
        bytebuffer/bytebuffer.cpp
        interpreter/YASL_string.cpp
        interpreter/refcount.cpp
        interpreter/list.cpp
        interpreter/userdata.cpp
        prime/prime.cpp
        compiler/env.cpp)

add_library(yaslapi
        yasl.cpp
        compiler/ast.cpp
        bytebuffer/bytebuffer.cpp
        compiler/compiler.cpp
        compiler/env.cpp
        compiler/lexer.cpp
        compiler/lexinput.cpp
        compiler/parser.cpp
        compiler/middleend.cpp
        hashtable/hashtable.cpp
        interpreter/bool_methods.cpp
        interpreter/builtins.cpp
        interpreter/float_methods.cpp
        interpreter/yasl_float.cpp
        interpreter/int_methods.cpp
        interpreter/list.cpp
        interpreter/list_methods.cpp
        interpreter/table_methods.cpp
        interpreter/VM.cpp
        interpreter/YASL_Object.cpp
        interpreter/refcount.cpp
        interpreter/str_methods.cpp
        interpreter/YASL_string.cpp
        interpreter/userdata.cpp
        interpreter/undef_methods.cpp
        prime/prime.cpp)

target_link_libraries(YASL m)
target_link_libraries(YASLTEST m)
target_link_libraries(yaslapi m)
