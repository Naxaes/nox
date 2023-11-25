#include "interpreter.h"
#include "code_generator/disassembler.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#define STACK_MAX_SIZE 1024
#define REG_MAX_SIZE 32

#define reg(interpreter, reg) interpreter.registers + reg; if (reg >= REG_MAX_SIZE) return (InterpreterResult) { 0, 1 }

void interpreter_free(Interpreter interpreter) {
    dealloc(interpreter.stack);
    dealloc(interpreter.registers);
}

InterpreterResult interpret(Bytecode code) {
    Interpreter interpreter = {
        .ip = 0,
        .stack = memset(alloc(sizeof(u64) * STACK_MAX_SIZE), 0, sizeof(u64) * STACK_MAX_SIZE),
        .stack_size = 1024,
        .registers = memset(alloc(sizeof(u64) * REG_MAX_SIZE), 0, sizeof(u64) * REG_MAX_SIZE),
        .instructions = code.instructions,
        .instructions_size = code.size,
    };

    while (interpreter.ip < interpreter.instructions_size) {
        Instruction instruction = interpreter.instructions[interpreter.ip];

        /*
        fprintf(stdout, "[%04zx]: ", interpreter.ip);
        disassemble_instruction(instruction, stdout);

        fprintf(stdout, " | bp=%llx, sp=%llx, ", interpreter.registers[0], interpreter.registers[1]);
        for (size_t i = 2; i < 8; ++i) {
            fprintf(stdout, "%lld, ", *reg(&interpreter, i));
        }
        fprintf(stdout, " | ");
        for (size_t i = 0; i < 8; ++i) {
            fprintf(stdout, "%lld, ", interpreter.stack[i]);
        }
        fprintf(stdout, "\n");
        */

        interpreter.ip++;
        u64* bp = &interpreter.registers[0];
        u64* sp = &interpreter.registers[1];

        switch (instruction.type) {
            case Instruction_MovImm64: {
                u64 dst = instruction.imm.dst;
                u64 val = instruction.imm.val;

                u64* r0 = reg(interpreter, dst);
                *r0 = val;
            } break;
            case Instruction_Mov: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r1;
            } break;
            case Instruction_Add: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 += *r1;
            } break;
            case Instruction_Add_Imm: {
                Register dst = instruction.imm.dst;
                Register val = instruction.imm.val;

                u64* r0 = reg(interpreter, dst);
                *r0 += val;
            } break;
            case Instruction_Sub: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 -= *r1;
            } break;
            case Instruction_Mul: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 *= *r1;
            } break;
            case Instruction_Div: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 /= *r1;
            } break;
            case Instruction_Mod: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 %= *r1;
            } break;
            case Instruction_Lt: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 < *r1;
            } break;
            case Instruction_Le: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 <= *r1;
            } break;
            case Instruction_Eq: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 == *r1;
            } break;
            case Instruction_Ne: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 != *r1;
            } break;
            case Instruction_Ge: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 >= *r1;
            } break;
            case Instruction_Gt: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 > *r1;
            } break;
            case Instruction_Not: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = !(*r1);
            } break;
            case Instruction_Neg: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = -(*r1);
            } break;
            case Instruction_And: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 && *r1;
            } break;
            case Instruction_Or: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                u64* r1 = reg(interpreter, src);
                *r0 = *r0 || *r1;
            } break;
            case Instruction_Store: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, src);
                interpreter.stack[*bp + dst] = *r0;
            } break;
            case Instruction_StoreAddr: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                interpreter.stack[*bp + dst] = *bp + src;
            } break;
            case Instruction_Load: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                u64* r0 = reg(interpreter, dst);
                *r0 = interpreter.stack[*bp + src];
            } break;
            case Instruction_Jmp: {
                interpreter.ip = instruction.jmp.label;
            } break;
            case Instruction_JmpZero: {
                Register src = instruction.jmp.src;
                u64* r0 = reg(interpreter, src);
                if (*r0 == 0) {
                    interpreter.ip = instruction.jmp.label;
                }
            } break;
            case Instruction_Print: {
                u64 value = interpreter.registers[2];
                printf("%s\n", (const char*) value);
            } break;
            case Instruction_Call: {
                interpreter.stack[*sp] = interpreter.ip;
                (*sp)++;
                interpreter.ip = instruction.call.label;
            } break;
            case Instruction_Ret: {
                interpreter.ip = interpreter.stack[--(*sp)];
            } break;
            case Instruction_Push: {
                Register src = instruction.reg.src;
                assert(*sp < STACK_MAX_SIZE && "Stack overflow");
                u64* r0 = reg(interpreter, src);
                interpreter.stack[(*sp)++] = *r0;
            } break;
            case Instruction_Pop: {
                Register dst = instruction.reg.dst;
                assert(*sp > 0 && "Stack underflow");
                u64* r0 = reg(interpreter, dst);
                *r0 = interpreter.stack[--(*sp)];
            } break;
            case Instruction_Exit: {
                assert(interpreter.registers[0] == 0 && interpreter.registers[1] == 0 && "Cleanup stack before exiting");
                i64 value = (i64) interpreter.registers[2];
                interpreter_free(interpreter);
                return (InterpreterResult) { value, 0 };
            } break;
            default: {
                fprintf(stderr, "[WARN] (Interpreter): Invalid instruction '%d'\n", instruction.type);
                interpreter_free(interpreter);
                return (InterpreterResult) { 0, 1 };
            } break;
        }
    }

    interpreter_free(interpreter);
    return (InterpreterResult) { 0, 1 };
}
