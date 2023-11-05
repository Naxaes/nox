#pragma once

#include "types.h"
#include "type_checker/checker.h"


typedef enum {
    Instruction_Invalid,
    Instruction_MovImm64,
    Instruction_Add,
    Instruction_Exit,
} Instruction;

typedef u32 Register;

typedef struct {
    u8*    instructions;
    size_t size;
} Bytecode;

Bytecode generate_code(TypedAst ast);

