#include "interpreter.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


i64 interpret(Bytecode code) {
    Interpreter interpreter = {
        .ip = 0,
        .stack = malloc(sizeof(u64) * 1024),
        .stack_size = 1024,
        .registers = malloc(sizeof(u64) * 1024),
        .registers_size = 1024,
        .instructions = code.instructions,
        .instructions_size = code.size,
    };

    while (interpreter.ip < interpreter.instructions_size) {
        Instruction instruction = interpreter.instructions[interpreter.ip++];
        switch (instruction.type) {
            case Instruction_Invalid: {
                fprintf(stderr, "[WARN]: (Interpreter) Invalid instruction '%d'\n", instruction.type);
                free(interpreter.stack);
                free(interpreter.registers);
                return 1;
            } break;
            case Instruction_MovImm64: {
                Register register_index = instruction.arg1;
                u64 value = instruction.arg2;
                interpreter.registers[register_index] = value;
            } break;
            case Instruction_Mov: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[src];
            } break;
            case Instruction_Add: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] += interpreter.registers[src];
            } break;
            case Instruction_Sub: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] -= interpreter.registers[src];
            } break;
            case Instruction_Mul: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] *= interpreter.registers[src];
            } break;
            case Instruction_Div: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] /= interpreter.registers[src];
            } break;
            case Instruction_Mod: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] %= interpreter.registers[src];
            } break;
            case Instruction_Lt: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[dst] < interpreter.registers[src];
            } break;
            case Instruction_Le: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[dst] <= interpreter.registers[src];
            } break;
            case Instruction_Eq: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[dst] == interpreter.registers[src];
            } break;
            case Instruction_Ne: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[dst] != interpreter.registers[src];
            } break;
            case Instruction_Ge: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[dst] >= interpreter.registers[src];
            } break;
            case Instruction_Gt: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[dst] > interpreter.registers[src];
            } break;
            case Instruction_Store: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[src];
            } break;
            case Instruction_Load: {
                Register dst = instruction.arg1;
                Register src = instruction.arg2;
                interpreter.registers[dst] = interpreter.registers[src];
            } break;
            case Instruction_Jmp: {
                interpreter.ip = instruction.arg1;
            } break;
            case Instruction_JmpZero: {
                Register src = instruction.arg1;
                u64 value = interpreter.registers[src];
                if (value == 0) {
                    interpreter.ip = instruction.arg2;
                }
            } break;
            case Instruction_Print: {
                Register src = instruction.arg1;
                printf("%s\n", (const char*) interpreter.registers[src]);
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
