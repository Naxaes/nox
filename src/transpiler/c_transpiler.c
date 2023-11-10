#include "c_transpiler.h"

#include <stdio.h>
#include <assert.h>


void transpile_instructions(Bytecode code, FILE* file, size_t from, size_t to) {
    for (size_t i = from; i < to; ++i) {
        Instruction instruction = code.instructions[i];
        switch (instruction) {
            case Instruction_Invalid: {
                fprintf(stderr, "[WARN]: Invalid instruction\n");
                return;
            } break;
            case Instruction_MovImm64: {
                u8 reg = code.instructions[++i];

                u64 value = 0;
                value |= (u64) code.instructions[++i] << 0;
                value |= (u64) code.instructions[++i] << 8;
                value |= (u64) code.instructions[++i] << 16;
                value |= (u64) code.instructions[++i] << 24;
                value |= (u64) code.instructions[++i] << 32;
                value |= (u64) code.instructions[++i] << 40;
                value |= (u64) code.instructions[++i] << 48;
                value |= (u64) code.instructions[++i] << 56;

                if (value >= 1uLL << 32) {
                    printf("[ERROR]: mov only supports 32-bit immediate\n");
                    return;
                }

                fprintf(file, "\treg[%d] = %llu;\n", reg, value);
            } break;
            case Instruction_Mov: {
                // int n = m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d];\n", dst, src);
            } break;
            case Instruction_Add: {
                // int n = n + m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] + reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Sub: {
                // int n = n - m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] - reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Mul: {
                // int n = n * m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] * reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Div: {
                // int n = n / m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] / reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Mod: {
                // int n = n % m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] %% reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Lt: {
                // int n = n < m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] < reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Le: {
                // int n = n <= m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] <= reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Eq: {
                // int n = n == m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] == reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Ne: {
                // int n = n != m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] != reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Ge: {
                // int n = n >= m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] >= reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Gt: {
                // int n = n > m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] > reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Store: {
                // int n = m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d];\n", dst, src);
            } break;
            case Instruction_Load: {
                // int n = m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d];\n", dst, src);
            } break;
            case Instruction_JmpZero: {
                // if (n == 0) goto label;
                u8 reg   = code.instructions[++i];
                u8 label = code.instructions[++i];

                fprintf(file, "\tif (reg[%d] == 0) goto label_%d;\n", reg, label);
                transpile_instructions(code, file, i + 1, label);
                fprintf(file, "\tlabel_%d:;\n", label);
            } break;
            case Instruction_Exit: {
                fprintf(file, "\treturn reg[0];\n");
                fprintf(file, "}\n");
            } break;
            default: {
                fprintf(stderr, "[ERROR] (Transpiler): Invalid instruction '%d'\n", instruction);
                return;
            } break;
        }
    }
}

void compile_c(Bytecode code) {
#if !defined(OUTPUT_C)
    return;
#else
    FILE* file = fopen(OUTPUT_C ".c", "w");
    if (!file) {
        fprintf(stderr, "[ERROR]: Could not open file\n");
        return;
    }

    fprintf(file, "int main(void) {\n");
    fprintf(file, "\tint reg[8] = { 0 };\n");

    transpile_instructions(code, file, 0, code.size);


#endif
}
