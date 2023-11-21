#include "types.h"

#include <stdio.h>

static inline u32 aarch64_mov_imm(u32 dest, u64 value) {
    if (value >= 1uLL << 13) {
        fprintf(stderr, "[WARN]: mov only supports 13-bit immediate\n");
        return 0;
    }

    // MOV Xd, #<imm>
    u32 rd  = dest & 0b1111;
    u32 imm = (value << 5) & 0b11111111111100000;
    u32 op  = 0b11010010100000000000000000000000;

    u32 inst = op | imm | rd;
    return inst;
}

static inline u32 aarch64_add(u32 dst, u32 src) {
    // ADD Xd, Xn, Xm
    u32 rd  = (dst & 0b11111) << 0;
    u32 rn  = (dst & 0b11111) << 5;
    u32 rm  = (src & 0b11111) << 16;
    u32 op  =  0b10001011000000000000000000000000;

    u32 inst = op | rm | rn | rd;
    return inst;
}

static inline u32 aarch64_mul(u32 dst, u32 src) {
    // MUL Xd, Xn, Xm
    u32 rd  = (dst & 0b11111) << 0;
    u32 rn  = (dst & 0b11111) << 5;
    u32 rm  = (src & 0b11111) << 16;
    u32 op  =  0b10011011000000000111110000000000;

    u32 inst = op | rm | rn | rd;
    return inst;
}

static inline u32 aarch64_mov_reg(u32 dest, u64 value) {
    // MOV Xd, Xn
    u32 rn  = (dest  & 0b11111) << 0;
    u32 rm  = (value & 0b11111) << 16;

    u32 op  =  0b10101010000000000000001111100000;

    u32 inst = op | rn | rm;
    return inst;
}

static inline u32 aarch64_ret(void) {
    // RET X30
    u32 inst = 0xd65f03c0;
    return inst;
}

