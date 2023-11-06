cmake_minimum_required(VERSION 3.14)

project(fuzzing VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined,leak,signed-integer-overflow")
set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -g -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined,leak,signed-integer-overflow")

add_executable(fuzzing main.c ../src/lexer/lexer.c)
target_include_directories(fuzzing PRIVATE ${PROJECT_SOURCE_DIR}/../src)
