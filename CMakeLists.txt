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
    set(CMAKE_C_FLAGS "/WX")
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
        data-structures/YASL_ByteBuffer.c
        compiler/compiler.c
        compiler/env.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/parser.c
        compiler/middleend.c
        data-structures/hash_function.c
        data-structures/YASL_Table.c
        interpreter/bool_methods.c
        interpreter/builtins.c
        interpreter/float_methods.c
        interpreter/yasl_float.c
        interpreter/int_methods.c
        data-structures/YASL_List.c
        interpreter/list_methods.c
        interpreter/table_methods.c
        interpreter/VM.c
        interpreter/YASL_Object.c
        interpreter/refcount.c
        interpreter/str_methods.c
        data-structures/YASL_String.c
        interpreter/userdata.c
        interpreter/undef_methods.c
        prime/prime.c
        std-io/yasl-std-io.c
        std-math/yasl-std-math.c
        std-require/yasl-std-require.c
        data-structures/YASL_Set.c
        std-collections/yasl-std-collections.c)

add_executable(YASLTEST
        test/yats.c
        test/yasl_test.c
        test/unit_tests/test_lexer/lexertest.c
        test/unit_tests/test_compiler/compilertest.c
        test/unit_tests/test_compiler/binoptest.c
        test/unit_tests/test_compiler/unoptest.c
        test/unit_tests/test_compiler/literaltest.c
        test/unit_tests/test_compiler/iftest.c
        test/unit_tests/test_compiler/whiletest.c
        test/unit_tests/test_compiler/fortest.c
        test/unit_tests/test_compiler/foreachtest.c
        test/unit_tests/test_compiler/foldingtest.c
        test/unit_tests/test_compiler/comprehensiontest.c
        test/unit_tests/test_collections/collectiontest.c
        test/unit_tests/test_collections/settest.c
        test/unit_tests/test_methods/methodtest.c
        test/unit_tests/test_methods/listtest.c
        test/unit_tests/test_methods/strtest.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/compiler.c
        compiler/parser.c
        compiler/ast.c
        compiler/middleend.c
        data-structures/hash_function.c
        data-structures/YASL_Table.c
        interpreter/yasl_float.c
        interpreter/YASL_Object.c
        data-structures/YASL_ByteBuffer.c
        data-structures/YASL_String.c
        interpreter/refcount.c
        data-structures/YASL_List.c
        interpreter/userdata.c
        prime/prime.c
        data-structures/YASL_Set.c
        compiler/env.c)

add_library(yaslapi
        yasl.c
        compiler/ast.c
        data-structures/YASL_ByteBuffer.c
        compiler/compiler.c
        compiler/env.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/parser.c
        compiler/middleend.c
        data-structures/hash_function.c
        data-structures/YASL_Table.c
        interpreter/bool_methods.c
        interpreter/builtins.c
        interpreter/float_methods.c
        interpreter/yasl_float.c
        interpreter/int_methods.c
        data-structures/YASL_List.c
        interpreter/list_methods.c
        interpreter/table_methods.c
        interpreter/VM.c
        interpreter/YASL_Object.c
        interpreter/refcount.c
        interpreter/str_methods.c
        data-structures/YASL_String.c
        interpreter/userdata.c
        interpreter/undef_methods.c
        prime/prime.c)

if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*MSVC.*")
    target_link_libraries(YASL m)
    target_link_libraries(YASLTEST m)
    target_link_libraries(yaslapi m)
endif()
