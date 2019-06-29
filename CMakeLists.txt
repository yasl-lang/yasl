cmake_minimum_required(VERSION 3.5.0)
project(YASL)

if(cpp)
    message(STATUS "COMPILING AS C++")
    file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/*.c")
    SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX )
endif()

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set("/WX")
endif()

if (NOT "${CMAKE_C_COMPILER_ID}" MATCHES ".*MSVC.*")
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-result -Wno-unused-parameter -Wpedantic -Werror")
endif()
if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*MSVC.*")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-result -Wno-unused-variable -Wno-unused-parameter -Wno-deprecated -Werror")
endif()
set(CMAKE_VERBOSE_MAKEFILE OFF)

include_directories(.)
include_directories(std-io)
include_directories(std-math)
include_directories(std-require)
include_directories(std-collections)
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
        hash_function/hash_function.c
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
        std-math/yasl-std-math.c
        std-require/yasl-std-require.c
        std-collections/set.c
        std-collections/yasl-std-collections.c)

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
        test/test_collections/collectiontest.c
        test/test_collections/settest.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/compiler.c
        compiler/parser.c
        compiler/ast.c
        compiler/middleend.c
        hash_function/hash_function.c
        hashtable/hashtable.c
        interpreter/yasl_float.c
        interpreter/YASL_Object.c
        bytebuffer/bytebuffer.c
        interpreter/YASL_string.c
        interpreter/refcount.c
        interpreter/list.c
        interpreter/userdata.c
        prime/prime.c
        std-collections/set.c
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
        hash_function/hash_function.c
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

if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*MSVC.*")
    target_link_libraries(YASL m)
    target_link_libraries(YASLTEST m)
    target_link_libraries(yaslapi m)
endif()
