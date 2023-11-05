cmake_minimum_required(VERSION 3.22)

project(nox VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(tests)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")

    # Disable warnings for fopen, strcpy, etc.
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-c11-extensions -Wno-gnu-binary-literal")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -fsanitize=signed-integer-overflow,address,undefined")  # memory
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
endif()

set(SOURCES
    src/lexer/lexer.c
    src/parser/parser.c
    src/type_checker/checker.c
    src/code_generator/generator.c
    src/interpreter/interpreter.c
    src/file.c
    src/str.c
    src/jit_compiler/jit.c
    src/os/memory.c)
add_executable(nox src/main.c ${SOURCES})
target_include_directories(nox PRIVATE src/)
target_compile_definitions(nox PRIVATE OUTPUT_JIT="jit")

