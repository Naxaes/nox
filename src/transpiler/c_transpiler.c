#include "c_transpiler.h"
#include "logger.h"

#include <stdio.h>
#include <assert.h>


int transpile_instructions(Bytecode code, FILE* file, size_t from, size_t to) {
    u32 labels[256] = { -1 };
    u32 label_i = 0;
    u32 label_count = 0;

    for (size_t i = from; i < to; ++i) {
        if (labels[label_i] == i) {
            fprintf(file, "\tlabel_%d:;\n", labels[label_i++]);
        }

        Instruction instruction = code.instructions[i];
        switch (instruction.type) {
            case Instruction_MovImm64: {
                u8  dst = instruction.imm.dst;
                u64 val = instruction.imm.val;
                
                fprintf(file, "\treg[%d] = %llu;\n", dst, val);
            } break;
            case Instruction_Mov: {
                // int n = m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d];\n", dst, src);
            } break;
            case Instruction_Add: {
                // int n = n + m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] + reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Add_Imm: {
                // int n = n + <val>;
                Register dst = instruction.imm.dst;
                i64      val = instruction.imm.val;

                fprintf(file, "\treg[%lld] = reg[%lld] + %lld;\n", dst, dst, val);
            } break;
            case Instruction_Sub: {
                // int n = n - m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] - reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Mul: {
                // int n = n * m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] * reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Div: {
                // int n = n / m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] / reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Mod: {
                // int n = n % m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] %% reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Lt: {
                // int n = n < m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] < reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Le: {
                // int n = n <= m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] <= reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Eq: {
                // int n = n == m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] == reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Ne: {
                // int n = n != m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] != reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Ge: {
                // int n = n >= m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] >= reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Gt: {
                // int n = n > m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d] > reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Store: {
                // int n = m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d];\n", dst, src);
            } break;
            case Instruction_Load: {
                // int n = m;
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;

                fprintf(file, "\treg[%d] = reg[%d];\n", dst, src);
            } break;
            case Instruction_Jmp: {
                // goto label;
                u8 label = instruction.jmp.label;

                fprintf(file, "\tgoto label_%d;\n", label);
                labels[label_count++] = label;
            } break;
            case Instruction_JmpZero: {
                // if (n == 0) goto label;
                u8 label = instruction.jmp.label;
                u8 src   = instruction.jmp.src;

                fprintf(file, "\tif (reg[%d] == 0) goto label_%d;\n", src, label);
                labels[label_count++] = label;
            } break;
            case Instruction_Exit: {
                fprintf(file, "\treturn reg[2];\n");
                fprintf(file, "}\n");
                return 0;
            } break;
            default: {
                error(0, "Invalid instruction '%d'\n", instruction.type);
                return -1;
            } break;
        }
    }

    return 0;
}

void compile_c(Bytecode code) {
#if !defined(OUTPUT_C)
    return;
#else
    FILE* file = fopen(OUTPUT_C ".c", "w");
    if (!file) {
        error(0, "Could not open file\n");
        return;
    }

    fprintf(file, "int main(void) {\n");
    fprintf(file, "\tint reg[8] = { 0 };\n");

    transpile_instructions(code, file, 0, code.size);


#endif
}
