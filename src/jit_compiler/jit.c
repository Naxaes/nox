#include "jit.h"

#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>


u32 aarch64_mov(u32 dest, u64 value) {
    if (value >= 1uLL << 13) {
        fprintf(stderr, "[ERROR]: mov only supports 13-bit immediate\n");
        return 0;
    }

    // MOV Xd, #<imm>
    u32 rd  = dest & 0b1111;
    u32 imm = (value << 5) & 0b11111111111100000;
    u32 op  = 0b11010010100000000000000000000000;

    u32 inst = op | imm | rd;
    return inst;
}

u32 aarch64_ret(void) {
    // RET X30
    u32 inst = 0xd65f03c0;
    return inst;
}


JitFunction jit_compile_aarch64(Bytecode code) {
    u32* machine_code = malloc(code.size);
    size_t size = 0;

    for (size_t i = 0; i < code.size; ++i) {
        Instruction instruction = code.instructions[i];
        switch (instruction) {
            case Instruction_Invalid: {
                fprintf(stderr, "Invalid instruction\n");
                return NULL;
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

                u32 inst = aarch64_mov(reg, value);
                if (inst == 0)
                    return NULL;
                machine_code[size++] = inst;
            } break;
            case Instruction_Exit: {
                u32 inst = aarch64_ret();
                machine_code[size++] = inst;
            } break;
            default: {
                fprintf(stderr, "Unknown instruction\n");
                return NULL;
            } break;
        }
    }

    void* memory = mmap(NULL, code.size, PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    memcpy(memory, machine_code, code.size);
    if (mprotect(memory, code.size, PROT_READ|PROT_EXEC) == -1) {
        perror("mprotect");
        return NULL;
    }

    return (JitFunction) memory;
}



// x86 register numbering is a bit bizarre:
// Number:    0,   1,   2,   3,   4,   5,   6,   7,
// Register: eax, ecx, edx, ebx, esp, ebp, esi, edi
#define x86_64_mov_imm32(reg, value) {  \
    0xb8 + (reg & 0b111),               \
    (value >> 0)  & 0xFF,               \
    (value >> 8)  & 0xFF,               \
    (value >> 16) & 0xFF,               \
    (value >> 24) & 0xFF,               \
}

#define x86_64_ret() {                  \
    0xc3                                \
}


JitFunction jit_compile_x86_64(Bytecode code) {
    u8* machine_code = malloc(code.size);
    size_t size = 0;

    for (size_t i = 0; i < code.size; ++i) {
        Instruction instruction = code.instructions[i];
        switch (instruction) {
            case Instruction_Invalid: {
                fprintf(stderr, "Invalid instruction\n");
                return NULL;
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
                    return NULL;
                }

                u8 inst[] = x86_64_mov_imm32(reg, value);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Exit: {
                u8 inst = x86_64_ret();
                machine_code[size++] = inst;
            } break;
            default: {
                printf("Unknown instruction\n");
                return NULL;
            } break;
        }
    }

    void* memory = mmap(NULL, code.size, PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    memcpy(memory, machine_code, code.size);
    if (mprotect(memory, code.size, PROT_READ|PROT_EXEC) == -1) {
        perror("mprotect");
        return NULL;
    }

    return (JitFunction) memory;
}



JitFunction jit_compile(Bytecode code) {
#if defined(__x86_64__)
    printf("[INFO]: JIT compiling for x86_64\n");
    return jit_compile_x86_64(code);
#elif defined(__aarch64__)
    printf("[INFO]: JIT compiling for aarch64\n");
    return jit_compile_aarch64(code);
#else
    return NULL;
#endif
}




