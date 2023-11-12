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
            fprintf(output, "%-10s r%-9llx #%-9llx", "Mov", instruction.imm.dst, instruction.imm.val);
        } break;
        case Instruction_Mov: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Mov", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Add: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Add", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Sub: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Sub", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Mul: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Mul", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Div: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Div", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Mod: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Mod", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Lt: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Lt", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Le: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Le", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Eq: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Eq", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Ne: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Ne", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Ge: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Ge", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Gt: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Gt", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Store: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Store", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Load: {
            fprintf(output, "%-10s r%-9llx r%-9llx", "Load", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Jmp: {
            fprintf(output, "%-10s [%04llx]%-4s %-10s", "Jmp", instruction.jmp.label, " ", " ");
        } break;
        case Instruction_JmpZero: {
            fprintf(output, "%-10s [%04llx]%-4s r%-9llx", "JmpZero", instruction.jmp.label, " ", instruction.jmp.src);
        } break;
        case Instruction_Print: {
            fprintf(output, "%-10s l%-9llx %-10s", "Print", instruction.call.label, " ");
        } break;
        case Instruction_Exit: {
            fprintf(output, "%-10s %-10s %-10s", "Exit", " ", " ");
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
        if (instruction.type == Instruction_Jmp || instruction.type == Instruction_JmpZero) {
            u64 start = (i < instruction.jmp.label) ? i : instruction.jmp.label;
            u64 stop  = (i < instruction.jmp.label) ? instruction.jmp.label : i;
            Label label = { .start = start, .stop = stop, .source = i, .target = instruction.jmp.label };
            disassembler.labels[disassembler.label_count++] = label;
        }
    }


    for (size_t i = 0; i < code.size; i++) {
        Instruction instruction = code.instructions[i];
        fprintf(output, "[%04zu]:  ", i);
        disassemble_instruction(instruction, output);
        fprintf(output, "  ");

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
