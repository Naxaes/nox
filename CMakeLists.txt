cmake_minimum_required(VERSION 3.22)

project(nox VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (BUILD_FUZZER)
    add_subdirectory(fuzzing)
endif()

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")

    # Disable warnings for fopen, strcpy, etc.
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-c11-extensions -Wno-gnu-binary-literal ")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")  # -fsanitize=fuzzer,address,undefined,leak,signed-integer-overflow")  # memory
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
endif()

set(SOURCES
    src/lexer/lexer.c
    src/lexer/token.c
    src/parser/parser.c
    src/type_checker/checker.c
    src/code_generator/generator.c
    src/code_generator/disassembler.c
    src/interpreter/interpreter.c
    src/logger.c
    src/file.c
    src/str.c
    src/args.c
    src/utf8.c
    src/error.c
    src/lib.c
    src/jit_compiler/jit.c
    src/os/memory.c
    src/transpiler/c_transpiler.c
    src/parser/visitor.c
    src/parser/ast_printer.c
    src/parser/node.c
    src/parser/tree.c
)
add_executable(nox src/main.c ${SOURCES})
target_include_directories(nox PRIVATE src/)
target_compile_definitions(nox PRIVATE OUTPUT_JIT="build/jit")
target_compile_definitions(nox PRIVATE OUTPUT_C="build/c")

