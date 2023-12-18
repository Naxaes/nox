#pragma once

#include "os/memory.h"
#include "types.h"
#include "type_checker/checker.h"

#include <stdlib.h>


#define ALL_INSTRUCTIONS(X) \
    X(Nop)               \
    X(MovImm64)          \
    X(Mov)               \
    X(Add)               \
    X(Add_Imm)           \
    X(Sub)               \
    X(Mul)               \
    X(Div)               \
    X(Mod)               \
    X(Lt)                \
    X(Le)                \
    X(Eq)                \
    X(Ne)                \
    X(Ge)                \
    X(Gt)                \
    X(Not)               \
    X(Neg)               \
    X(And)               \
    X(Or)                \
    X(Store)             \
    X(StoreAddr)         \
    X(Load)              \
    X(Jmp)               \
    X(JmpZero)           \
    X(Push)              \
    X(Pop)               \
    X(Print)             \
    X(Call)              \
    X(Ret)               \
    X(Exit)              \

typedef enum {
#define X(name) Instruction_##name,
    ALL_INSTRUCTIONS(X)
#undef X
    INSTRUCTION_LAST = Instruction_Exit,
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

static inline void bytecode_free(Bytecode code) {
    dealloc(code.instructions);
}

Bytecode generate_code(TypedAst ast);

