#include "disassembler.h"

#include <assert.h>

typedef struct {
    u16 start;
    u16 stop;
    u16 source;
    u16 target;
} Label;

typedef struct {
    Bytecode code;
    FILE* output;
    Label labels[12];
    u32   label_count;
} Disassembler;


void disassemble_instruction(Instruction instruction, FILE* output) {
    switch (instruction.type) {
        case Instruction_MovImm64: {
            fprintf(output, "%-10s r%-7llu #%-7llu", "Mov", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Mov: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Mov", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Add: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Add", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Sub: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Sub", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Mul: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Mul", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Div: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Div", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Mod: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Mod", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Lt: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Lt", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Le: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Le", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Eq: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Eq", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Ne: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Ne", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Ge: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Ge", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Gt: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Gt", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Store: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Store", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Load: {
            fprintf(output, "%-10s r%-7llu r%-7llu", "Load", instruction.arg1, instruction.arg2);
        } break;
        case Instruction_Jmp: {
            fprintf(output, "%-10s [%04llu]%-2s %-8s", "Jmp", instruction.arg1, " ", " ");
        } break;
        case Instruction_JmpZero: {
            fprintf(output, "%-10s r%-7llu [%04llu]%-2s", "JmpZero", instruction.arg1, instruction.arg2, " ");
        } break;
        case Instruction_Print: {
            fprintf(output, "%-10s l%-7llu %-8s", "Print", instruction.arg1, " ");
        } break;
        case Instruction_Exit: {
            fprintf(output, "%-10s %-8s %-8s", "Exit", " ", " ");
        } break;
        default: {
            assert(0 && "Invalid instruction type");
        } break;
    }
}

void disassemble(Bytecode code, FILE* output) {
    Disassembler disassembler = { .code = code, .output = output };

    for (size_t i = 0; i < code.size; i++) {
        Instruction instruction = code.instructions[i];
        if (instruction.type == Instruction_Jmp) {
            u64 start = (i < instruction.arg1) ? i : instruction.arg1;
            u64 stop  = (i < instruction.arg1) ? instruction.arg1 : i;
            Label label = { .start = start, .stop = stop, .source = i, .target = instruction.arg1 };
            disassembler.labels[disassembler.label_count++] = label;
        } else if (instruction.type == Instruction_JmpZero) {
            u64 start = (i < instruction.arg2) ? i : instruction.arg2;
            u64 stop  = (i < instruction.arg2) ? instruction.arg2 : i;
            Label label = { .start = start, .stop = stop, .source = i, .target = instruction.arg2 };
            disassembler.labels[disassembler.label_count++] = label;
        }
    }


    for (size_t i = 0; i < code.size; i++) {
        Instruction instruction = code.instructions[i];
        fprintf(output, "[%04zu]:  ", i);
        disassemble_instruction(instruction, output);

        for (size_t j = 0; j < disassembler.label_count; j++) {
            Label label = disassembler.labels[j];
            if (label.target == i) {
                if (label.source < label.target)
                    fprintf(output, "<┘ ");
                else if (label.target < label.source) {
                    fprintf(output, "<┐ ");
                }
            } else if (label.start < i && i < label.stop) {
                fprintf(output, " │ ");
            } else if (label.source == i) {
                if (label.source < label.target)
                    fprintf(output, "─┐ ");
                else if (label.target < label.source) {
                    fprintf(output, "─┘ ");
                }
            } else {
                fprintf(output, "   ");
            }
        }

        fprintf(output, "\n");
    }
}
