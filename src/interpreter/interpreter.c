#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "interpreter.h"
#include "code_generator/disassembler.h"
#include "allocator.h"



#define STACK_MAX_SIZE 1024
#define REG_MAX_SIZE 32

i64 interpret(Bytecode code) {
    Interpreter interpreter = {
        .ip = 0,
        .stack = alloc(0, sizeof(u64) * STACK_MAX_SIZE),
        .stack_size = 1024,
        .registers = alloc(0, sizeof(u64) * REG_MAX_SIZE),
        .instructions = code.instructions,
        .instructions_size = code.size,
    };
    memset(interpreter.stack, 0, sizeof(u64) * STACK_MAX_SIZE);
    memset(interpreter.registers, 0, sizeof(u64) * REG_MAX_SIZE);

    while (interpreter.ip < interpreter.instructions_size) {
        Instruction instruction = interpreter.instructions[interpreter.ip];


        /*
        fprintf(stdout, "[%04zx]: ", interpreter.ip);
        disassemble_instruction(instruction, stdout);

        fprintf(stdout, " | bp=%llx, sp=%llx, ", interpreter.registers[0], interpreter.registers[1]);
        for (size_t i = 2; i < 8; ++i) {
            if (interpreter.registers[i] > 0xFFFFFF)
                fprintf(stdout, "'%s', ", (const char*)interpreter.registers[i]);
            else
                fprintf(stdout, "%lld, ", interpreter.registers[i]);
        }
        fprintf(stdout, " | ");
        for (size_t i = 0; i < 8; ++i) {
            if (interpreter.stack[i] > 0xFFFFFF)
                fprintf(stdout, "'%s', ", (const char*)interpreter.stack[i]);
            else
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
            case Instruction_Add_Imm: {
                Register dst = instruction.imm.dst;
                Register val = instruction.imm.val;
                interpreter.registers[dst] += val;
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
                interpreter.stack[*bp + dst] = interpreter.registers[src];
            } break;
            case Instruction_Load: {
                Register dst = instruction.reg.dst;
                Register src = instruction.reg.src;

                interpreter.registers[dst] = interpreter.stack[*bp + src];
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
                u64 value = interpreter.stack[*(sp)-1];
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
                interpreter.stack[(*sp)++] = interpreter.registers[src];
            } break;
            case Instruction_Pop: {
                Register dst = instruction.reg.dst;
                assert(*sp > 0 && "Stack underflow");
                interpreter.registers[dst] = interpreter.stack[--(*sp)];
            } break;
            case Instruction_Exit: {
                assert(interpreter.registers[0] == 0 && interpreter.registers[1] == 0 && "Cleanup stack before exiting");
                i64 value = (i64) interpreter.registers[2];
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
