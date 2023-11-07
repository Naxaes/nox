#include "c_transpiler.h"

#include <stdio.h>
#include <assert.h>


void generate_name(FILE* out, u8 reg) {
    fprintf(out, "reg_%c", 'a' + reg);
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

    for (size_t i = 0; i < code.size; ++i) {
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
            case Instruction_Add: {
                // int n = n + m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] + reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Mul: {
                // int n = n * m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d] * reg[%d];\n", dst, dst, src);
            } break;
            case Instruction_Exit: {
                fprintf(file, "\treturn reg[0];\n");
                fprintf(file, "}\n");
            } break;
            case Instruction_Store: {
                // int n = m;
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];

                fprintf(file, "\treg[%d] = reg[%d];\n", dst, src);
            } break;
        }
    }
#endif
}
