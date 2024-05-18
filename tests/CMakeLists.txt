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


include_directories(${PROJECT_SOURCE_DIR}/../src)

enable_testing()
set(SOURCES
    ${PROJECT_SOURCE_DIR}/../src/lexer/token.c
    ${PROJECT_SOURCE_DIR}/../src/lexer/lexer.c
    ${PROJECT_SOURCE_DIR}/../src/file.c
    ${PROJECT_SOURCE_DIR}/../src/lib.c
    ${PROJECT_SOURCE_DIR}/../src/allocator.c
    ${PROJECT_SOURCE_DIR}/../src/str.c
    ${PROJECT_SOURCE_DIR}/../src/args.c
    ${PROJECT_SOURCE_DIR}/../src/utf8.c
    ${PROJECT_SOURCE_DIR}/../src/error.c
    ${PROJECT_SOURCE_DIR}/../src/os/memory.c
    ${PROJECT_SOURCE_DIR}/../src/lexer/lexer.c
    ${PROJECT_SOURCE_DIR}/../src/parser/parser.c
    ${PROJECT_SOURCE_DIR}/../src/parser/ast_printer.c
    ${PROJECT_SOURCE_DIR}/../src/type_checker/checker.c
    ${PROJECT_SOURCE_DIR}/../src/code_generator/generator.c
    ${PROJECT_SOURCE_DIR}/../src/code_generator/disassembler.c
    ${PROJECT_SOURCE_DIR}/../src/interpreter/interpreter.c
    ${PROJECT_SOURCE_DIR}/../src/jit_compiler/jit.c
    ${PROJECT_SOURCE_DIR}/../src/transpiler/c_transpiler.c
)
message(STATUS "SOURCES: ${SOURCES}")


add_executable(main ${SOURCES} main.cpp)
target_include_directories(main PRIVATE ${SRC_DIR})
target_link_libraries(main GTest::gtest_main GTest::gmock_main)

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
