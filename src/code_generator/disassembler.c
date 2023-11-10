#include "disassembler.h"

#include <assert.h>


void disassemble_instruction(Instruction instruction, FILE* output) {
    switch (instruction.type) {
        case Instruction_Invalid: {
            fprintf(output, "Invalid\n");
        } break;
        case Instruction_MovImm64: {
            fprintf(output, "%-10s r%-7llu #%-7llu\n", "Mov", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Mov: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Mov", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Add: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Add", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Sub: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Sub", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Mul: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Mul", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Div: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Div", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Mod: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Mod", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Lt: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Lt", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Le: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Le", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Eq: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Eq", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Ne: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Ne", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Ge: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Ge", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Gt: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Gt", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Store: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Store", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Load: {
            fprintf(output, "%-10s r%-7llu r%-7llu\n", "Load", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Jmp: {
            fprintf(output, "%-10s [%04llu]\n", "Jmp", instruction.arg1);
        } break;
        case Instruction_JmpZero: {
            fprintf(output, "%-10s r%-7llu [%04llu]\n", "JmpZero", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Print: {
            fprintf(output, "%-10s l%-7llu\n", "Print", instruction.arg1);
        } break;
        case Instruction_Exit: {
            fprintf(output, "%-10s\n", "Exit");
        } break;
        default: {
            assert(0 && "Invalid instruction type");
        } break;
    }
}

void disassemble(Bytecode code, FILE* output) {
    for (size_t i = 0; i < code.size; i++) {
        Instruction instruction = code.instructions[i];
        fprintf(output, "[%04zu]:  ", i);
        disassemble_instruction(instruction, output);
    }
}
