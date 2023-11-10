#pragma once

#include "types.h"
#include "type_checker/checker.h"


typedef enum {
    Instruction_Invalid,
    Instruction_MovImm64,
    Instruction_Mov,
    Instruction_Add,
    Instruction_Sub,
    Instruction_Mul,
    Instruction_Div,
    Instruction_Mod,
    Instruction_Lt,
    Instruction_Le,
    Instruction_Eq,
    Instruction_Ne,
    Instruction_Ge,
    Instruction_Gt,
    Instruction_Store,
    Instruction_Load,
    Instruction_Jmp,
    Instruction_JmpZero,
    Instruction_Print,
    Instruction_Exit,
} InstructionType;

typedef struct {
    InstructionType type;
    u64 arg1;
    u64 arg2;
} Instruction;

typedef u32 Register;

typedef struct {
    Instruction* instructions;
    size_t size;
} Bytecode;

Bytecode generate_code(TypedAst ast);

