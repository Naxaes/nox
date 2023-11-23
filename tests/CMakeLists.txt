cmake_minimum_required(VERSION 3.14)

project(test VERSION 0.1 LANGUAGES C CXX)

# Use the new policy behaviour for CMP0135 (to suppress warning)
cmake_policy(SET CMP0135 NEW)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/../src)

enable_testing()
set(SOURCES
    ${SRC_DIR}/lexer/lexer.c
    ${SRC_DIR}/lexer/token.c
    ${SRC_DIR}/parser/parser.c
    ${SRC_DIR}/type_checker/checker.c
    ${SRC_DIR}/code_generator/generator.c
    ${SRC_DIR}/code_generator/disassembler.c
    ${SRC_DIR}/interpreter/interpreter.c
    ${SRC_DIR}/logger.c
    ${SRC_DIR}/file.c
    ${SRC_DIR}/str.c
    ${SRC_DIR}/args.c
    ${SRC_DIR}/utf8.c
    ${SRC_DIR}/error.c
    ${SRC_DIR}/memory.c
    ${SRC_DIR}/lib.c
    ${SRC_DIR}/jit_compiler/jit.c
    ${SRC_DIR}/os/memory.c
    ${SRC_DIR}/transpiler/c_transpiler.c
    ${SRC_DIR}/parser/visitor.c
    ${SRC_DIR}/parser/ast_printer.c
    ${SRC_DIR}/parser/node.c
)
add_executable(lexer ${SOURCES} lexer.cpp)
target_include_directories(lexer PRIVATE ${SRC_DIR})
target_link_libraries(lexer GTest::gtest_main GTest::gmock_main)

add_executable(arithmetic ${SOURCES} arithmetic.cpp)
target_include_directories(arithmetic PRIVATE ${SRC_DIR})
target_link_libraries(arithmetic GTest::gtest_main GTest::gmock_main)

add_executable(functions ${SOURCES} functions.cpp)
target_include_directories(functions PRIVATE ${SRC_DIR})
target_link_libraries(functions GTest::gtest_main GTest::gmock_main)

add_executable(if-stmt ${SOURCES} if-stmt.cpp)
target_include_directories(if-stmt PRIVATE ${SRC_DIR})
target_link_libraries(if-stmt GTest::gtest_main GTest::gmock_main)

add_executable(while-stmt ${SOURCES} while-stmt.cpp)
target_include_directories(while-stmt PRIVATE ${SRC_DIR})
target_link_libraries(while-stmt GTest::gtest_main GTest::gmock_main)

add_executable(logic ${SOURCES} logic.cpp)
target_include_directories(logic PRIVATE ${SRC_DIR})
target_link_libraries(logic GTest::gtest_main GTest::gmock_main)



include(GoogleTest)
gtest_discover_tests(lexer
    NO_PRETTY_TYPES
    EXCLUDE gtest gtest_main gmock gmock_main
)
