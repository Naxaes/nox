#include "disassembler.h"

#include <assert.h>

typedef struct {
    u64 call_target;
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
            fprintf(output, "%-10s r%-9x #%-9x", "Mov", instruction.imm.dst, instruction.imm.val);
        } break;
        case Instruction_Mov: {
            fprintf(output, "%-10s r%-9x r%-9x", "Mov", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Add: {
            fprintf(output, "%-10s r%-9x r%-9x", "Add", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Sub: {
            fprintf(output, "%-10s r%-9x r%-9x", "Sub", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Mul: {
            fprintf(output, "%-10s r%-9x r%-9x", "Mul", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Div: {
            fprintf(output, "%-10s r%-9x r%-9x", "Div", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Mod: {
            fprintf(output, "%-10s r%-9x r%-9x", "Mod", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Lt: {
            fprintf(output, "%-10s r%-9x r%-9x", "Lt", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Le: {
            fprintf(output, "%-10s r%-9x r%-9x", "Le", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Eq: {
            fprintf(output, "%-10s r%-9x r%-9x", "Eq", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Ne: {
            fprintf(output, "%-10s r%-9x r%-9x", "Ne", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Ge: {
            fprintf(output, "%-10s r%-9x r%-9x", "Ge", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Gt: {
            fprintf(output, "%-10s r%-9x r%-9x", "Gt", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Store: {
            fprintf(output, "%-10s r%-9x r%-9x", "Store", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Load: {
            fprintf(output, "%-10s r%-9x r%-9x", "Load", instruction.reg.dst, instruction.reg.src);
        } break;
        case Instruction_Jmp: {
            fprintf(output, "%-10s [%04x]%-4s %-10s", "Jmp", instruction.jmp.label, " ", " ");
        } break;
        case Instruction_JmpZero: {
            fprintf(output, "%-10s [%04x]%-4s r%-9x", "JmpZero", instruction.jmp.label, " ", instruction.jmp.src);
        } break;
        case Instruction_Print: {
            fprintf(output, "%-10s %-10s %-10s", "Print", " ", " ");
        } break;
        case Instruction_Call: {
            fprintf(output, "%-10s [%04x]%-4s %-10s", "Call", instruction.call.label, " ", " ");
        } break;
        case Instruction_Ret: {
            fprintf(output, "%-10s %-10s %-10s", "Ret", " ", " ");
        } break;
        case Instruction_Push: {
            fprintf(output, "%-10s r%-9x %-10s", "Push", instruction.reg.src, " ");
        } break;
        case Instruction_Pop: {
            fprintf(output, "%-10s r%-9x %-10s", "Pop", instruction.reg.dst, " ");
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

    for (i32 i = 0; i < (i32) code.size; i++) {
        Instruction instruction = code.instructions[i];
        if (instruction.type == Instruction_Jmp || instruction.type == Instruction_JmpZero || instruction.type == Instruction_Call) {
            u64 start = (i < instruction.jmp.label) ? i : instruction.jmp.label;
            u64 stop  = (i < instruction.jmp.label) ? instruction.jmp.label : i;
            Label label = { .call_target = instruction.type == Instruction_Call, .start = start, .stop = stop, .source = i, .target = instruction.jmp.label };
            disassembler.labels[disassembler.label_count++] = label;
        }
    }


    for (size_t i = 0; i < code.size; i++) {
        Instruction instruction = code.instructions[i];


        for (size_t j = 0; j < disassembler.label_count; j++) {
            Label label = disassembler.labels[j];
            if (label.target == i && label.call_target) {
                fprintf(output, "────────────────────────[%04zx]─────────────", i);
                for (size_t k = 0; k < disassembler.label_count; k++) {
                    if (disassembler.labels[k].start <= i && i <= disassembler.labels[k].stop) {
                        fprintf(output, "─│─");
                    } else {
                        fprintf(output, "───");
                    }
                }
                fprintf(output, "\n");
                break;
            }
        }


        fprintf(output, "[%04zx]:  ", i);
        disassemble_instruction(instruction, output);
        fprintf(output, "  ");

        for (size_t j = 0; j < disassembler.label_count; j++) {
            Label label = disassembler.labels[j];
            if (label.target == i) {
                if (label.source < label.target) {
                    fprintf(output, "<┘ ");
                }
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
