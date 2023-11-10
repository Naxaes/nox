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
        switch (instruction) {
            case Instruction_Invalid: {
                fprintf(stderr, "[WARN]: Invalid instruction\n");
                free(interpreter.stack);
                free(interpreter.registers);
                return 1;
            } break;
            case Instruction_MovImm64: {
                Register register_index = interpreter.instructions[interpreter.ip++];

                u64 value = 0;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 0;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 8;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 16;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 24;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 32;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 40;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 48;
                value |= (u64) interpreter.instructions[interpreter.ip++] << 56;

                interpreter.registers[register_index] = value;
            } break;
            case Instruction_Mov: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[source];
            } break;
            case Instruction_Add: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] += interpreter.registers[source];
            } break;
            case Instruction_Sub: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] -= interpreter.registers[source];
            } break;
            case Instruction_Mul: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] *= interpreter.registers[source];
            } break;
            case Instruction_Div: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] /= interpreter.registers[source];
            } break;
            case Instruction_Mod: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] %= interpreter.registers[source];
            } break;
            case Instruction_Lt: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[dest] < interpreter.registers[source];
            } break;
            case Instruction_Le: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[dest] <= interpreter.registers[source];
            } break;
            case Instruction_Eq: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[dest] == interpreter.registers[source];
            } break;
            case Instruction_Ne: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[dest] != interpreter.registers[source];
            } break;
            case Instruction_Ge: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[dest] >= interpreter.registers[source];
            } break;
            case Instruction_Gt: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[dest] > interpreter.registers[source];
            } break;
            case Instruction_Store: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[source];
            } break;
            case Instruction_Load: {
                Register dest   = interpreter.instructions[interpreter.ip++];
                Register source = interpreter.instructions[interpreter.ip++];
                interpreter.registers[dest] = interpreter.registers[source];
            } break;
            case Instruction_Jmp: {
                interpreter.ip = interpreter.instructions[interpreter.ip];
            } break;
            case Instruction_JmpZero: {
                Register register_index = interpreter.instructions[interpreter.ip++];
                u64 value = interpreter.registers[register_index];
                if (value == 0) {
                    interpreter.ip = interpreter.instructions[interpreter.ip];
                } else {
                    interpreter.ip++;
                }
            } break;
            case Instruction_Print: {
                Register register_index = interpreter.instructions[interpreter.ip++];
                printf("%s\n", (const char*) interpreter.registers[register_index]);
            } break;
            case Instruction_Exit: {
                i64 value = (i64) interpreter.registers[0];
                return value;
            } break;
            default: {
                fprintf(stderr, "[WARN] (Interpreter): Invalid instruction\n");
                free(interpreter.stack);
                free(interpreter.registers);
                return 1;
            } break;
        }
    }

    return 0;
}
