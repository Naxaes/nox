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


enable_testing()
set(SOURCES
    ${PROJECT_SOURCE_DIR}/../src/lexer/lexer.c
    ${PROJECT_SOURCE_DIR}/../src/file.c
    ${PROJECT_SOURCE_DIR}/../src/str.c
    ${PROJECT_SOURCE_DIR}/../src/args.c
    ${PROJECT_SOURCE_DIR}/../src/utf8.c
    ${PROJECT_SOURCE_DIR}/../src/error.c
    ${PROJECT_SOURCE_DIR}/../src/os/memory.c
)
message(STATUS "SOURCES: ${SOURCES}")
add_executable(lexer ${SOURCES} lexer.cpp)
target_include_directories(lexer PRIVATE ${PROJECT_SOURCE_DIR}/../src)
target_link_libraries(lexer GTest::gtest_main GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(lexer
    NO_PRETTY_TYPES
    EXCLUDE gtest gtest_main gmock gmock_main
)
