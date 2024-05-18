#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "jit.h"
#include "allocator.h"

#include "os/memory.h"
#include "aarch64.h"
#include "x86_64.h"



JitFunction jit_compile_aarch64(Bytecode code) {
    u32* machine_code = alloc(0, code.size * sizeof(*machine_code));
    size_t size = 0;

    for (size_t i = 0; i < code.size; ++i) {
        Instruction instruction = code.instructions[i];
        switch (instruction.type) {
            case Instruction_MovImm64: {
                u8  dst = instruction.imm.dst;
                u64 val = instruction.imm.val;

                u32 inst = aarch64_mov_imm(dst, val);
                if (inst == 0)
                    return NULL;
                machine_code[size++] = inst;
            } break;
            case Instruction_Mov: {
                // MOV Xd, Xn
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u32 inst = aarch64_mov_reg(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Add: {
                // ADD Xd, Xn, Xm
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u32 inst = aarch64_add(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Mul: {
                // MUL Xd, Xn, Xm
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u32 inst = aarch64_mul(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Store: {
                // MOV Xd, Xn, Xm
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u32 inst = aarch64_mov_reg(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Load: {
                // MOV Xd, Xn, Xm
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u32 inst = aarch64_mov_reg(dst, src);
                machine_code[size++] = inst;
            } break;
            case Instruction_Exit: {
                u32 inst = aarch64_ret();
                machine_code[size++] = inst;
            } break;
            default: {
                warn(0, "Invalid instruction '%d'", instruction.type);
                return NULL;
            } break;
        }
    }

    void* memory = memory_map_executable(machine_code, size * sizeof(*machine_code));
    return (JitFunction) memory;
}


JitFunction jit_compile_x86_64(Bytecode code) {
    u8* machine_code = alloc(0, code.size);
    size_t size = 0;

    for (size_t i = 0; i < code.size; ++i) {
        Instruction instruction = code.instructions[i];
        switch (instruction.type) {
            case Instruction_MovImm64: {
                u64 dst = instruction.imm.dst;
                u64 val = instruction.imm.val;

                if (val >= 1uLL << 32) {
                    error(0, "mov only supports 32-bit immediate\n");
                    return NULL;
                }

                u8 inst[] = x86_64_mov_imm32(dst, val);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Mov: {
                // movl %enx %enx
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u8 inst[] = x86_64_mov_reg(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Add: {
                // addl %enx %enx
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u8 inst[] = x86_64_add(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Mul: {
                // imul %enx %enx
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u8 inst[] = x86_64_mul(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Store: {
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u8 inst[] = x86_64_mov_reg(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
             } break;
            case Instruction_Load: {
                u8 dst = instruction.reg.dst;
                u8 src = instruction.reg.src;
                u8 inst[] = x86_64_mov_reg(dst, src);
                memcpy(&machine_code[size], inst, sizeof(inst));
                size += sizeof(inst);
            } break;
            case Instruction_Exit: {
                u8 inst = x86_64_ret();
                machine_code[size++] = inst;
            } break;
            default: {
                warn(0, "Invalid instruction '%d'\n", instruction.type);
                return NULL;
            } break;
        }
    }

    void* memory = memory_map_executable(machine_code, size * sizeof(*machine_code));
    return (JitFunction) memory;
}



JitFunction jit_compile(Bytecode code, int output) {
#if defined(__x86_64__)
    if (output) info(0, "JIT compiling for x86_64");
    JitFunction function =  jit_compile_x86_64(code);
    if (function == NULL)
        return NULL;
#elif defined(__aarch64__)
    if (output) info(0, "[INFO]: JIT compiling for aarch64");
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
        info(0, "Disassembly of " OUTPUT_JIT);
        info(0, "%s", buffer);
    }
#endif
#endif

    return function;
}




