#include "jit.h"

#include "os/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


u32 aarch64_mov_imm(u32 dest, u64 value) {
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

u32 aarch64_add(u32 dst, u32 src) {
    // ADD Xd, Xn, Xm
    u32 rd  = (dst & 0b11111) << 0;
    u32 rn  = (dst & 0b11111) << 5;
    u32 rm  = (src & 0b11111) << 16;
    u32 op  =  0b10001011000000000000000000000000;

    u32 inst = op | rm | rn | rd;
    return inst;
}

u32 aarch64_mul(u32 dst, u32 src) {
    // MUL Xd, Xn, Xm
    u32 rd  = (dst & 0b11111) << 0;
    u32 rn  = (dst & 0b11111) << 5;
    u32 rm  = (src & 0b11111) << 16;
    u32 op  =  0b10011011000000000111110000000000;

    u32 inst = op | rm | rn | rd;
    return inst;
}

u32 aarch64_mov_reg(u32 dest, u64 value) {
    // MOV Xd, Xn
    u32 rn  = (dest  & 0b11111) << 0;
    u32 rm  = (value & 0b11111) << 16;

    u32 op  =  0b10101010000000000000001111100000;

    u32 inst = op | rn | rm;
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
                fprintf(stderr, "[WARN]: Invalid instruction\n");
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

                u32 inst = aarch64_mov_imm(reg, value);
                if (inst == 0)
                    return NULL;
                machine_code[size++] = inst;
            } break;
            case Instruction_Mov: {
                // MOV Xd, Xn
                u8 dst = code.instructions[++i];
                u8 src = code.instructions[++i];
                u32 inst = aarch64_mov_reg(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Add: {
                // ADD Xd, Xn, Xm
                u8 dst = code.instructions[++i];
                u8 src = code.instructions[++i];
                u32 inst = aarch64_add(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Mul: {
                // MUL Xd, Xn, Xm
                u8 dst = code.instructions[++i];
                u8 src = code.instructions[++i];
                u32 inst = aarch64_mul(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Store: {
                // MOV Xd, Xn, Xm
                u8 dst = code.instructions[++i];
                u8 src = code.instructions[++i];
                u32 inst = aarch64_mov_reg(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Load: {
                // MOV Xd, Xn, Xm
                u8 dst = code.instructions[++i];
                u8 src = code.instructions[++i];
                u32 inst = aarch64_mov_reg(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Exit: {
                u32 inst = aarch64_ret();
                machine_code[size++] = inst;
            } break;
            default: {
                fprintf(stderr, "[WARN]: Invalid instruction '%d'\n", instruction);
                return NULL;
            } break;
        }
    }

    void* memory = memory_map_executable(machine_code, size * sizeof(*machine_code));
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

#define x86_64_add(dst, src) {        \
    0x01,                               \
    0xc0 + ((src & 0b111) << 3) + (dst & 0b111), \
}

#define x86_64_mul(dst, src) {        \
    0x0f,                               \
    0xaf,                               \
    0xc0 + ((dst & 0b111) << 3) + (src & 0b111), \
}

#define x86_64_mov_reg(dst, src) {    \
    0x89,                               \
    0xc0 + ((src & 0b111) << 3) + (dst & 0b111), \
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
                fprintf(stderr, "[WARN]: Invalid instruction\n");
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
            case Instruction_Mov: {
                // movl %enx %enx
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];
                u8 inst[] = x86_64_mov_reg(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Add: {
                // addl %enx %enx
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];
                u8 inst[] = x86_64_add(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Mul: {
                // imul %enx %enx
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];
                u8 inst[] = x86_64_mul(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Store: {
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];
                u8 inst[] = x86_64_mov_reg(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
             } break;
            case Instruction_Load: {
                u8 dst   = code.instructions[++i];
                u8 src   = code.instructions[++i];
                u8 inst[] = x86_64_mov_reg(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Exit: {
                u8 inst = x86_64_ret();
                machine_code[size++] = inst;
            } break;
            default: {
                fprintf(stderr, "[WARN]: Invalid instruction '%d'\n", instruction);
                return NULL;
            } break;
        }
    }

    void* memory = memory_map_executable(machine_code, size * sizeof(*machine_code));
    return (JitFunction) memory;
}



JitFunction jit_compile(Bytecode code, int output) {
#if defined(__x86_64__)
    if (output) printf("[INFO]: JIT compiling for x86_64\n");
    JitFunction function =  jit_compile_x86_64(code);
    if (function == NULL)
        return NULL;
#elif defined(__aarch64__)
    if (output) printf("[INFO]: JIT compiling for aarch64\n");
    JitFunction function = jit_compile_aarch64(code);
    if (function == NULL)
        return NULL;
#else
    JitFunction function = NULL;
    return NULL;
#endif

#ifdef OUTPUT_JIT
    if (!output)
        return function;

    FILE* file = fopen(OUTPUT_JIT ".bin", "wb");
    assert(file != NULL && "Failed to open " OUTPUT_JIT);
    fwrite((void*)function, code.size, 1, file);
    fclose(file);

#if defined(__x86_64__)
    const char* command = "llvm-objcopy -I binary -O elf64-x86-64 --rename-section=.data=.text,code " OUTPUT_JIT ".bin " OUTPUT_JIT ".elf && objdump --disassemble " OUTPUT_JIT ".elf";
#elif defined(__aarch64__)
    const char* command = "llvm-objcopy -I binary -O elf64-aarch64 --rename-section=.data=.text,code " OUTPUT_JIT ".bin " OUTPUT_JIT ".elf && objdump --disassemble " OUTPUT_JIT ".elf";
#else
    const char* command = "";
    return function;
#endif

#if !defined(_WIN32)
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("popen");
        return function;
    }

    char buffer[1024] = { '\0' };
    char line[1024] = { '\0' };
    size_t size = 0;
    while (fgets(line, sizeof(line), pipe) != NULL) {
        strncat(buffer, line, sizeof(buffer) - size - 1);
        size += strnlen(line, sizeof(line) - size - 1);
    }

    // Close the pipe
    int status = pclose(pipe);
    if (status == 0) {
        printf("[INFO]: Disassembly of " OUTPUT_JIT);
        printf("%s\n", buffer);
    }
#endif
#endif

    return function;
}




