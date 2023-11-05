cmake_minimum_required(VERSION 3.14)

project(test VERSION 0.1 LANGUAGES C CXX)

# Use the new policy behaviour for CMP0135 (to suppress warning)
cmake_policy(SET CMP0135 NEW)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()
add_executable(lexer ../src/lexer/lexer.c lexer.cpp)
target_include_directories(lexer PRIVATE ${PROJECT_SOURCE_DIR}/../src)
target_link_libraries(lexer GTest::gtest_main GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(lexer
    NO_PRETTY_TYPES
    EXCLUDE gtest gtest_main gmock gmock_main
)
