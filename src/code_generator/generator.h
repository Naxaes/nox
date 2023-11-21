#pragma once

#include "types.h"
#include "type_checker/checker.h"


#define ALL_INSTRUCTIONS \
    X(MovImm64) \
    X(Mov) \
    X(Add) \
    X(Sub) \
    X(Mul) \
    X(Div) \
    X(Mod) \
    X(Lt) \
    X(Le) \
    X(Eq) \
    X(Ne) \
    X(Ge) \
    X(Gt) \
    X(Store) \
    X(Load) \
    X(Jmp) \
    X(JmpZero) \
    X(Print) \
    X(Exit)

typedef enum {
    Instruction_MovImm64,
    Instruction_Mov,
    Instruction_Add,
    Instruction_Add_Imm,
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
    Instruction_Push,
    Instruction_Pop,
    Instruction_Print,
    Instruction_Call,
    Instruction_Ret,
    Instruction_Exit,
} InstructionType;

typedef struct {
    InstructionType type;
    union {
        struct {
            i64 dst;
            i64 val;
        } imm;
        struct {
            i64 dst;
            i64 src;
        } reg;
        struct {
            i64 label;
            i64 src;
        } jmp;
        struct {
            i64 label;
        } call;
    };
} Instruction;

typedef i64 Register;

typedef struct {
    Instruction* instructions;
    size_t size;
} Bytecode;

Bytecode generate_code(TypedAst ast);

