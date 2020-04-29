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
    set(CMAKE_C_FLAGS "/WX /W3")
endif()

if (NOT "${CMAKE_C_COMPILER_ID}" MATCHES ".*MSVC.*")
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-result -Wno-unused-parameter -Wno-vla -Wpedantic -Werror")
endif()
if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*MSVC.*")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-result -Wno-unused-variable -Wno-unused-parameter -Wno-vla -Wno-deprecated -Werror")
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
        util/IO.c)

add_executable(yasltest
        opcode.h
        test/yats.c
        test/yasl_test.c
        test/unit_tests/test_api/apitest.c
        test/unit_tests/test_api/pushtest.c
        test/unit_tests/test_api/poptest.c
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
        test/unit_tests/test_compiler/syntaxerrortest.c
        test/unit_tests/test_collections/collectiontest.c
        test/unit_tests/test_collections/settest.c
        test/unit_tests/test_methods/methodtest.c
        test/unit_tests/test_methods/listtest.c
        test/unit_tests/test_methods/strtest.c
        test/unit_tests/test_vm/vmtest.c
        compiler/lexer.c
        compiler/lexinput.c
        compiler/compiler.c
        compiler/parser.c
        compiler/ast.c
        compiler/middleend.c
        util/hash_function.c
        data-structures/YASL_Table.c
        util/yasl_float.c
        interpreter/YASL_Object.c
        data-structures/YASL_ByteBuffer.c
        data-structures/YASL_String.c
        interpreter/refcount.c
        data-structures/YASL_List.c
        interpreter/userdata.c
        util/prime.c
        data-structures/YASL_Set.c
        compiler/env.c
        yasl.c
        interpreter/upvalue.c
        interpreter/VM.c
        interpreter/builtins.c
        interpreter/list_methods.c
        interpreter/table_methods.c
        interpreter/str_methods.c
        interpreter/undef_methods.c
        interpreter/float_methods.c
        interpreter/int_methods.c
        interpreter/bool_methods.c
        util/IO.c)

if (NOT "${CMAKE_C_COMPILER_ID}" MATCHES ".*MSVC.*")
    add_executable(tests
            opcode.h
            test/test.c
            test/clitest.c
            test/filetest.c
            test/memtest.c
            test/test_util.c)
endif()

add_library(yaslapi
        opcode.h
        yasl.c
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
        util/IO.c)

if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*MSVC.*")
    target_link_libraries(yasl m)
    target_link_libraries(yasltest m)
    target_link_libraries(yaslapi m)
    target_link_libraries(tests m)
endif()
