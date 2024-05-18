#include "preamble.h"


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
