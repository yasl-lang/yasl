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

# 4244 is lossy coercions:
# https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-levels-3-and-4-c4244
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS "/WX /W3 /wd4244")
endif()

if (NOT "${CMAKE_C_COMPILER_ID}" MATCHES ".*MSVC.*")
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-vla -Wno-logical-op-parentheses -Wno-parentheses -Werror")
endif()
if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*MSVC.*")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-vla -Wno-deprecated -Wno-logical-op-parentheses -Wno-parentheses -Werror")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pedantic-ms-format")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pedantic-ms-format")
endif()

set(CMAKE_VERBOSE_MAKEFILE OFF)

include_directories(.)
include_directories(compiler)
include_directories(data-structures)
include_directories(interpreter)
include_directories(util)
include_directories(std)
include_directories(test)

add_executable(yasl
        opcode.h
        yasl.c
        yasl_aux.c
        main.c
        compiler/ast.c
        data-structures/YASL_ByteBuffer.c
        compiler/compiler.c
        compiler/env.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/parser.c
        compiler/middleend.c
        interpreter/upvalue.c
        interpreter/closure.c
        util/hash_function.c
        data-structures/YASL_Table.c
        interpreter/bool_methods.c
        interpreter/builtins.c
        interpreter/float_methods.c
        util/yasl_float.c
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
        util/prime.c
        std/yasl-std-io.c
        std/yasl-std-math.c
        std/yasl-std-require.c
        data-structures/YASL_Set.c
        std/yasl-std-collections.c
        util/IO.c
        util/varint.c
        yasl_conf.h
        yasl_error.h
        yasl_plat.h
        std/yasl-std-mt.c std/yasl-std-mt.h)

add_library(yaslapi
        opcode.h
        yasl.c
        yasl_aux.c
        compiler/ast.c
        data-structures/YASL_ByteBuffer.c
        compiler/compiler.c
        compiler/env.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/parser.c
        compiler/middleend.c
        util/hash_function.c
        data-structures/YASL_Table.c
        interpreter/bool_methods.c
        interpreter/builtins.c
        interpreter/float_methods.c
        interpreter/upvalue.c
        interpreter/closure.c
        util/yasl_float.c
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
        util/prime.c
        util/IO.c
        util/varint.c
        std/yasl-std-collections.c
        std/yasl-std-io.c
        std/yasl-std-math.c
        std/yasl-std-require.c
        data-structures/YASL_Set.c
        std/yasl-std-mt.c std/yasl-std-mt.h)

add_executable(tests
        test2/integration_tests.c
        test/yats.c
        test/yasl_test.c
        test/unit_tests/test_api/apitest.c
        test/unit_tests/test_api/pushtest.c
        test/unit_tests/test_api/poptest.c
        test/unit_tests/test_lexer/lexertest.c
        test/unit_tests/test_env/envtest.c
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
        test/unit_tests/test_compiler/syntaxerrortest.c
        test/unit_tests/test_compiler/closuretest.c
        test/unit_tests/test_collections/collectiontest.c
        test/unit_tests/test_collections/settest.c
        test/unit_tests/test_methods/methodtest.c
        test/unit_tests/test_methods/listtest.c
        test/unit_tests/test_methods/strtest.c
        test/unit_tests/test_vm/vmtest.c
        test/unit_tests/test_util/utiltest.c
        test/unit_tests/test_api/deltest.c)

if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*MSVC.*")
    target_link_libraries(yasl m)
    target_link_libraries(yaslapi m)
    target_link_libraries(tests m)
endif()

target_link_libraries(tests yaslapi)

if (UNIX)
    target_link_libraries(yasl dl)
    target_link_libraries(yaslapi dl)
    target_link_libraries(tests dl)
endif()
