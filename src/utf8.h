#pragma once
#include "types.h"

#ifndef STRING_BOUNDS_CHECK
#define STRING_BOUNDS_CHECK
#endif

// NOTE(ted): This is undefined behaviour:
//      string.data[0] << 0  |
//      string.data[1] << 8  |
//      string.data[2] << 16 |
//      string.data[3] << 24;
// because char might be signed and can't be shifted
// left 24 times.
static inline u8 to_u8(char x) { return (u8) x; }
#define PACK_CHAR_ARRAY_TO_RUNE_2(x) (rune) ((to_u8((x)[0]) << 0u) | (to_u8((x)[1]) << 8u))
#define PACK_CHAR_ARRAY_TO_RUNE_3(x) (rune) ((to_u8((x)[0]) << 0u) | (to_u8((x)[1]) << 8u) | (to_u8((x)[2]) << 16u))
#define PACK_CHAR_ARRAY_TO_RUNE_4(x) (rune) ((to_u8((x)[0]) << 0u) | (to_u8((x)[1]) << 8u) | (to_u8((x)[2]) << 16u) | (to_u8((x)[3]) << 24u))
#define RUNE_AS_CHAR_ARRAY(x) (char[]) { (char)((x) >> 0), (char)((x) >> 8), (char)((x) >> 16), (char)((x) >> 24), 0 }

rune char_array_to_rune(const char* bytes);
int  is_continuation_byte(char byte);
int  is_valid_start_byte(char byte);
int  multi_byte_count(char byte);
int  rune_matches(const char* a, rune b);
int  is_whitespace(rune c);
int  matches(rune a, rune b, rune x, rune y);
int  is_alpha(rune c);
int  is_digit(rune a);

static inline int are_continuations(const char* chars, int count) {
    for (int i = 0; i < count; ++i)
        if (!is_continuation_byte(chars[i]))
            return 0;
    return 1;
}
