#pragma once

#include <string.h>
#include "preamble.h"

typedef struct {
    size_t size;
    const char* data;
} Str;

#define STR(literal) ((Str) { sizeof(literal)-1, (literal) })
#define STR_EMPTY ((Str) {0, NULL})

#define STR_FMT "%.*s"
#define STR_ARG(str) (int)(str).size, (str).data


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

static inline int str_equals(Str a, Str b) {
    if (a.size != b.size)
        return 0;
    return memcmp(a.data, b.data, a.size) == 0;
}

static inline Str str_from_c_str(const char* cstr) {
    return (Str) { strlen(cstr), cstr };
}

static inline Str str_line_at(Str source, size_t index) {
    size_t start = index;
    while (start > 0 && source.data[start-1] != '\n') {
        start -= 1;
    }

    size_t end = index;
    while (end < source.size && source.data[end] != '\n') {
        end += 1;
    }

    return (Str) { end - start, source.data + start };
}
