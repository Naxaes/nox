#pragma once

#include <string.h>
#include "types.h"

typedef struct {
    size_t size;
    const char* data;
} Str;

#define STR(literal) ((Str) { sizeof(literal)-1, (literal) })
#define STR_EMPTY ((Str) {0, NULL})


static inline int str_is_empty(Str str) {
    return str.size == 0;
}

static inline u64 str_hash(Str str) {
    u64 hash = 5381;
    for (size_t i = 0; i < str.size; ++i) {
        hash = ((hash << 5) + hash) + str.data[i];
    }
    return hash;
}

static inline int str_compare(Str a, Str b) {
    if (a.size != b.size)
        return (int)a.size - (int)b.size;
    return memcmp(a.data, b.data, a.size);
}
