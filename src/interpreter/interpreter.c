#include "interpreter.h"
#include "code_generator/disassembler.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>



i64 interpret(Bytecode code) {
    Interpreter interpreter = {
        .ip = 0,
        .stack = malloc(sizeof(u64) * 1024),
        .stack_size = 1024,
        .registers = malloc(sizeof(u64) * 8),
        .registers_size = 8,
        .instructions = code.instructions,
        .instructions_size = code.size,
    };

    u64* sp = interpreter.stack;


    while (interpreter.ip < interpreter.instructions_size) {
        Instruction instruction = interpreter.instructions[interpreter.ip];
//        fprintf(stdout, "[%-4zu]: ", interpreter.ip);
//        disassemble_instruction(instruction, stdout);
//        fprintf(stdout, "\n");
        interpreter.ip++;

        switch (instruction.type) {
            case Instruction_MovImm64: {
                u64 dst = instruction.imm.dst;
                u64 val = instruction.imm.val;
                interpreter.registers[dst] = val;
            } break;
            case Instruction_Mov: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[src];
            } break;
            case Instruction_Add: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] += interpreter.registers[src];
            } break;
            case Instruction_Sub: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] -= interpreter.registers[src];
            } break;
            case Instruction_Mul: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] *= interpreter.registers[src];
            } break;
            case Instruction_Div: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] /= interpreter.registers[src];
            } break;
            case Instruction_Mod: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] %= interpreter.registers[src];
            } break;
            case Instruction_Lt: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[dst] < interpreter.registers[src];
            } break;
            case Instruction_Le: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[dst] <= interpreter.registers[src];
            } break;
            case Instruction_Eq: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[dst] == interpreter.registers[src];
            } break;
            case Instruction_Ne: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[dst] != interpreter.registers[src];
            } break;
            case Instruction_Ge: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[dst] >= interpreter.registers[src];
            } break;
            case Instruction_Gt: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[dst] > interpreter.registers[src];
            } break;
            case Instruction_Store: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[src];
            } break;
            case Instruction_Load: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;
                interpreter.registers[dst] = interpreter.registers[src];
            } break;
            case Instruction_Jmp: {
                interpreter.ip = instruction.jmp.label;
            } break;
            case Instruction_JmpZero: {
                Register src = instruction.jmp.src;
                u64 value = interpreter.registers[src];
                if (value == 0) {
                    interpreter.ip = instruction.jmp.label;
                }
            } break;
            case Instruction_Print: {
                Register src = instruction.call.label;
                printf("%s\n", (const char*) interpreter.registers[src]);
            } break;
            case Instruction_Call: {
                *sp++ = interpreter.ip;
                interpreter.ip = instruction.call.label;
            } break;
            case Instruction_Ret: {
                interpreter.ip = *--sp;
            } break;
            case Instruction_Exit: {
                i64 value = (i64) interpreter.registers[0];
                return value;
            } break;
            default: {
                fprintf(stderr, "[WARN] (Interpreter): Invalid instruction '%d'\n", instruction.type);
                free(interpreter.stack);
                free(interpreter.registers);
                return 1;
            } break;
        }
    }

    return 0;
}
