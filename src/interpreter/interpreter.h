#pragma once

#include "types.h"
#include "code_generator/generator.h"

typedef struct {
    size_t ip;
    u64*   stack;
    size_t stack_size;
    u64*   registers;
    size_t registers_size;
    u8*    instructions;
    size_t instructions_size;
} Interpreter;


i64 interpret(Bytecode code);
