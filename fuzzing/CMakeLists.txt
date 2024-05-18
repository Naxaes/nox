cmake_minimum_required(VERSION 3.14)

project(fuzzing VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined,leak,signed-integer-overflow")
set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -g -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined,leak,signed-integer-overflow")

set(SOURCES
    ../src/lexer/token.c
    ../src/lexer/lexer.c
    ../src/parser/parser.c
    ../src/type_checker/checker.c
    ../src/code_generator/generator.c
    ../src/interpreter/interpreter.c
    ../src/allocator.c
    ../src/file.c
    ../src/str.c
    ../src/args.c
    ../src/utf8.c
    ../src/error.c
    ../src/jit_compiler/jit.c
    ../src/os/memory.c
)
add_executable(fuzzing main.c ${SOURCES})
target_include_directories(fuzzing PRIVATE ${PROJECT_SOURCE_DIR}/../src)
